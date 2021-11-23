#include "replicated_log_node/replicated_log_master.h"

void ReplicatedLogMaster::SetSecondaryNodesList(
    const std::string& secondary_nodes) {
  if (secondary_nodes.empty()) return;

  // TODO: Rethink splitting the string by delimiter
  std::vector<std::string> results;
  boost::split(results, secondary_nodes, [](char c) { return c == ','; });
  m_secondaries.clear();
  std::vector<std::string> secondary_splitted;
  for (const auto& result : results) {
    boost::split(secondary_splitted, result, [](char c) { return c == ':'; });
    m_secondaries.push_back(
        Secondary{secondary_splitted[0], secondary_splitted[1]});
  }
}

Mif::Net::Http::Code ReplicatedLogMaster::StoreMessage(
    const Json::Value& node) {
  const auto message_body = node["message"].asString();
  const std::size_t write_concern = node["write_concern"].asUInt();
  const auto message = InternalMessage(m_message_id, message_body);
  m_messages[m_message_id] = message;
  SendMessageToSecondaries(message, write_concern);
  m_message_id++;
  return Mif::Net::Http::Code::Ok;
}

void ReplicatedLogMaster::SendMessageToSecondaries(InternalMessage message,
                                                   std::size_t write_concern) {
  auto node_responce = SendMessages(message);

  std::chrono::milliseconds retry_delay;
  std::chrono::system_clock::time_point timeout;
  do {
    retry_delay = std::chrono::milliseconds(m_responce_timeout);
    timeout = std::chrono::system_clock::now() + retry_delay;
  } while (write_concern > GatherResponses(node_responce, timeout));
}

std::vector<ReplicatedLogMaster::NodeResponce>
ReplicatedLogMaster::SendMessages(InternalMessage message) {
  auto json_message = message.ToJson().toStyledString();
  std::vector<NodeResponce> node_response;
  node_response.reserve(m_secondaries.size());

  for (const auto secondary : m_secondaries) {
    std::string url_string = secondary.host + ":" + secondary.port;
    std::promise<cpr::Response> p1;
    std::future<cpr::Response> f_completes = p1.get_future();
    std::thread(
        [](std::promise<cpr::Response> p1, const std::string& url_str,
           const std::string& mesage_str) {
          cpr::Response r = cpr::Post(cpr::Url{url_str}, cpr::Body(mesage_str));
          p1.set_value_at_thread_exit(r);
        },
        std::move(p1), url_string, json_message)
        .detach();

    const bool kResponceReceived = false;
    node_response.emplace_back(
        NodeResponce{url_string, kResponceReceived, std::move(f_completes)});
  }
  return node_response;
}

std::size_t ReplicatedLogMaster::GatherResponses(
    std::vector<ReplicatedLogMaster::NodeResponce>& node_response,
    const std::chrono::system_clock::time_point& timeout) {
  std::size_t current_concert_level = 1;
  for (auto& responce : node_response) {
    if (responce.is_received) {
      current_concert_level++;
    } else if (std::future_status::ready ==
               responce.furute.wait_until(timeout)) {
      const auto http_response = responce.furute.get();
      responce.is_received = true;
      auto status_code = http_response.status_code;
      if (cpr::status::HTTP_OK == status_code) {
        current_concert_level++;
      } else {
        MIF_LOG(Info) << "secondary node " << responce.url
                      << " post message is not successfull. Status code "
                      << status_code;
      }
    } else {
      MIF_LOG(Info) << "secondary node " << responce.url
                    << " post message confirmation is not received";
    }
  }
  return current_concert_level;
};