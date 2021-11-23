#include "replicated_log_node/replicated_log_secondary.h"

void ReplicatedLogSecondary::SetResponceDelay(const std::size_t& delay) {
  m_response_delay_ms = delay;
}

Mif::Net::Http::Code ReplicatedLogSecondary::StoreMessage(
    const Json::Value& node) {
  std::this_thread::sleep_for(std::chrono::milliseconds(m_response_delay_ms));

  const std::size_t message_id = node["id"].asUInt();
  const auto message_body = node["message"].asString();
  auto id_position = m_messages.find(message_id);
  if (id_position == m_messages.end()) {
    m_messages[message_id] = InternalMessage(message_id, message_body);
  } else {
    // Nothing, message is present in database
  }
  return Mif::Net::Http::Code::Ok;
}
