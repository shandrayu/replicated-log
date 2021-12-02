#include "replicated_log_node/replicated_log_master.h"

std::string ReplicatedLogMaster::GetStatusStr(
    ReplicatedLogMaster::NodeHealth::Status status) const {
  std::string status_str;
  switch (status) {
    case ReplicatedLogMaster::NodeHealth::Status::Healthy:
      status_str = "Healthy";
      break;
    case ReplicatedLogMaster::NodeHealth::Status::Suspected:
      status_str = "Suspected";
      break;
    case ReplicatedLogMaster::NodeHealth::Status::Unhealthy:
      status_str = "Unhealthy";
      break;
    default:
      status_str = "Unhealthy";
      break;
  }
  return status_str;
}

ReplicatedLogMaster::Secondary::Secondary(const std::string& host,
                                          const std::string& port)
    : m_host(host), m_port(port) {
  m_url = host + ":" + port;
}

void ReplicatedLogMaster::NodeHealth::Update(const std::int32_t cpr_status) {
  if (cpr::status::HTTP_OK == cpr_status) {
    status = Status::Healthy;
  } else if (Status::Healthy == status) {
    status = Status::Suspected;
  } else {
    status = Status::Unhealthy;
  }
}

bool ReplicatedLogMaster::NodeHealth::isOk() const {
  return NodeHealth::Status::Healthy == status ||
         NodeHealth::Status::Suspected == status;
}

ReplicatedLogMaster::ReplicatedLogMaster() {
  m_health_check_thread = std::thread([this]() {
    do {
      for (const auto& secondary : m_secondaries) {
        const cpr::Response responce =
            cpr::Get(cpr::Url{secondary.GetUrl() + "/health"},
                     cpr::Timeout{m_responce_timeout});

        std::unique_lock<std::shared_mutex> lock(m_secondary_status_mutex);
        if (m_secondary_health.empty()) {
          continue;
        }

        m_secondary_health.at(secondary.GetHash()).Update(responce.status_code);
        const auto status = m_secondary_health.at(secondary.GetHash()).status;
        MIF_LOG(Info) << "[Health] Secondary " << secondary.GetUrl()
                      << ": health status is " << GetStatusStr(status);
        if (NodeHealth::Status::Healthy == status ||
            NodeHealth::Status::Suspected == status) {
          lock.unlock();
          m_secondary_health.at(secondary.GetHash()).cv->notify_all();
        }
      }

      std::this_thread::sleep_for(
          std::chrono::milliseconds(m_health_check_period_ms));
    } while (!m_terminate_health_status_check);
  });
}

ReplicatedLogMaster::~ReplicatedLogMaster() {
  m_terminate_health_status_check = true;
  if (m_health_check_thread.joinable()) {
    m_health_check_thread.join();
  }
}

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
    auto secondary = Secondary{secondary_splitted[0], secondary_splitted[1]};
    m_secondaries.emplace_back(secondary);
    m_secondary_health.try_emplace(secondary.GetHash(), NodeHealth());
  }
}

void ReplicatedLogMaster::SetHealthCheckPeriod(const std::size_t period_ms) {
  m_health_check_period_ms = period_ms;
}

void ReplicatedLogMaster::EnableRetry(bool enable) { m_retry = enable; }

Mif::Net::Http::Code ReplicatedLogMaster::StoreMessage(
    const Json::Value& node) {
  if (!HasQuorum()) {
    return Mif::Net::Http::Code::NotModified;
  }

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
    // TODO: capture this. It will fail if program will be closed before the
    // thread has finished
    std::thread(
        [this](std::shared_ptr<CountDownLatch> countdown,
               const std::string& url_str, const std::string& message_str,
               const int message_id, bool retry_if_failed) {
          cpr::Response responce;
          do {
            {
              std::shared_lock<std::shared_mutex> lock(
                  m_secondary_status_mutex);
              if (NodeHealth::Status::Healthy !=
                  m_secondary_health.at(url_str).status) {
                // wait for successful status notification
                m_secondary_health.at(url_str).cv->wait(lock);
              } else {
                // can proceed with sending the message
              }
            }

            MIF_LOG(Info) << "Try to send message id=" << message_id << " to "
                          << url_str;
            responce = cpr::Post(cpr::Url{url_str}, cpr::Body(message_str),
                                 cpr::Timeout{m_responce_timeout});

          } while (cpr::status::HTTP_OK != responce.status_code &&
                   retry_if_failed);
          if (cpr::status::HTTP_OK == responce.status_code) {
            MIF_LOG(Info) << "Message id=" << message_id << " to " << url_str
                          << " sent!";
            countdown->count_down();
          } else {
            // TODO: not possible in this implementation
            MIF_LOG(Info) << "Message id=" << message_id << " to " << url_str
                          << " is not sent!";
          }
        },
        countdown, secondary.GetUrl(), json_message, message.id, m_retry)
        .detach();
  }
  // TODO: Implement wait with timeout?
  countdown->await(/*timeout*/);
}

bool ReplicatedLogMaster::HasQuorum() const {
  const std::size_t num_quorum = m_secondaries.size() / 2 + 1;
  std::size_t active_nodes = 1;

  std::shared_lock<std::shared_mutex> lock(m_secondary_status_mutex);
  for (const auto& node : m_secondary_health) {
    if (node.second.isOk()) {
      active_nodes++;
    }
  }
  return active_nodes >= num_quorum;
}
