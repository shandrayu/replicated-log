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
  const int write_concern = node["write_concern"].asInt();
  InternalMessage message;
  {
    std::lock_guard<std::mutex> lck(m_message_queue_mutex);
    message = InternalMessage(m_message_id, message_body);
    m_messages[m_message_id++] = message;
  }

  SendMessageToSecondaries(message, write_concern);
  return Mif::Net::Http::Code::Ok;
}

void ReplicatedLogMaster::SendMessageToSecondaries(InternalMessage message,
                                                   int write_concern) {
  auto json_message = message.ToJson().toStyledString();

  auto countdown = std::make_shared<CountDownLatch>(write_concern - 1);
  for (const auto& secondary : m_secondaries) {
    std::string url_string = secondary.host + ":" + secondary.port;
    std::thread(
        [](std::shared_ptr<CountDownLatch> countdown,
           const std::string& url_str, const std::string& mesage_str) {
          cpr::Response responce =
              cpr::Post(cpr::Url{url_str}, cpr::Body(mesage_str));
          if (cpr::status::HTTP_OK == responce.status_code) {
            countdown->count_down();
          } else {
            // In case is we received error responce from the node we cannot
            // increase counter
            MIF_LOG(Info) << "secondary node " << url_str
                          << " post message is not successfull. Status code "
                          << responce.status_code;
          }
        },
        countdown, url_string, json_message)
        .detach();
  }
  // TODO: Implement wait with timeout?
  countdown->await(/*timeout*/);
}
