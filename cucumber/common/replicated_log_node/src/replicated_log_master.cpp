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

void ReplicatedLogMaster::EnableRetry(bool enable) { m_retry = enable; }

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
           const std::string& url_str, const std::string& message_str,
           const int message_id, bool retry_if_failed) {
          cpr::Response responce;
          std::size_t response_delay_ms = 1;
          int responce_timeout_ms = 5000;
          do {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(response_delay_ms));
            MIF_LOG(Info) << "Try to send message id=" << message_id << " to "
                          << url_str << " with response_delay "
                          << response_delay_ms << " ms";
            responce = cpr::Post(cpr::Url{url_str}, cpr::Body(message_str),
                                 cpr::Timeout{responce_timeout_ms});
            response_delay_ms *= 2;
          } while (cpr::status::HTTP_OK != responce.status_code &&
                   retry_if_failed);
          if (cpr::status::HTTP_OK == responce.status_code) {
            MIF_LOG(Info) << "Message id=" << message_id << " to " << url_str
                          << " sent!";
            countdown->count_down();
          } else {
            MIF_LOG(Info) << "Message id=" << message_id << " to " << url_str
                          << " is not sent!";
          }
        },
        countdown, url_string, json_message, message.id, m_retry)
        .detach();
  }
  // TODO: Implement wait with timeout?
  countdown->await(/*timeout*/);
}
