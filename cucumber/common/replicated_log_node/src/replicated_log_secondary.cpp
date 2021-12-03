#include "replicated_log_node/replicated_log_secondary.h"

void ReplicatedLogSecondary::HealthStatusRequestHandler(
    Mif::Net::Http::IInputPack const& request,
    Mif::Net::Http::IOutputPack& response) {
  if (request.GetType() == Mif::Net::Http::Method::Type::Get) {
    auto const data = request.GetData();
    MIF_LOG(Info) << "[Health] Status request: Start processing...";
    MIF_LOG(Info) << "[Health] Status request: Done!";
    response.SetCode(Mif::Net::Http::Code::Ok);
  }
}

void ReplicatedLogSecondary::SetResponceDelay(const std::size_t delay) {
  m_response_delay_ms = delay;
}

void ReplicatedLogSecondary::SetServerErrorMessageId(const int id) {
  m_server_error.message_id = id;
}

Mif::Net::Http::Code ReplicatedLogSecondary::StoreMessage(
    const Json::Value& node) {
  std::this_thread::sleep_for(std::chrono::milliseconds(m_response_delay_ms));
  {
    std::lock_guard<std::mutex> lck(m_message_queue_mutex);
    const std::size_t message_id = node["id"].asUInt();
    if (m_server_error.message_id == message_id && !m_server_error.triggered) {
      // Skip message with this ID only once
      m_server_error.triggered = true;
      return Mif::Net::Http::Code::Internal;
    }
    const auto message_body = node["message"].asString();
    auto id_position = m_messages.find(message_id);
    if (id_position == m_messages.end()) {
      m_messages[message_id] = InternalMessage(message_id, message_body);
    } else {
      // Nothing, message is present in database
    }
  }

  return Mif::Net::Http::Code::Ok;
}
