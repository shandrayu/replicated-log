#include "replicated_log_node/node_health_checher.h"

#include <cpr/cpr.h>
#include <mif/common/log.h>

NodeHealthChecker::~NodeHealthChecker() { Reset(); }

void NodeHealthChecker::Setup(const std::vector<Secondary>& secondaries) {
  if (secondaries.empty()) {
    return;
  }

  m_terminate_health_status_check = false;
  const auto health_check_function = [this](const std::string& secondary_url,
                                            const std::string& secondary_hash) {
    do {
      const cpr::Response responce =
          cpr::Get(cpr::Url{secondary_url + "/health"},
                   cpr::Timeout{m_health_check_period_ms / 2});

      std::unique_lock<std::shared_mutex> lock(m_health_status_mutex);
      auto& node_health = m_health_status.at(secondary_hash);
      node_health.Update(responce.status_code);
      MIF_LOG(Info) << "[Health] Secondary " << secondary_url
                    << ": health status is " << node_health.GetStatusStr();
      if (node_health.isOk()) {
        lock.unlock();
        node_health.cv->notify_all();
      }

      std::this_thread::sleep_for(
          std::chrono::milliseconds(m_health_check_period_ms));
    } while (!m_terminate_health_status_check);
  };

  for (const auto& secondary : secondaries) {
    m_health_status[secondary.GetHash()].thread = std::move(std::thread(
        health_check_function, secondary.GetUrl(), secondary.GetHash()));
  }
}

void NodeHealthChecker::Reset() {
  m_terminate_health_status_check = true;
  for (auto& status : m_health_status) {
    if (status.second.thread.joinable()) {
      status.second.thread.join();
    }
  }
}

void NodeHealthChecker::SetHealthCheckPeriod(const std::size_t period_ms) {
  m_health_check_period_ms = period_ms;
}

void NodeHealthChecker::WaitOkStatus(const std::string secondary_hash) {
  std::shared_lock<std::shared_mutex> lock(m_health_status_mutex);
  const auto& node_health = m_health_status.at(secondary_hash);
  if (Status::Healthy != node_health.status) {
    // wait for successful status notification
    node_health.cv->wait(lock);
  } else {
    // can proceed with sending the message
  }
}

bool NodeHealthChecker::HasQuorum() const {
  const std::size_t num_quorum = m_health_status.size() / 2 + 1;
  std::size_t active_nodes = 1;

  std::shared_lock<std::shared_mutex> lock(m_health_status_mutex);
  for (const auto& node : m_health_status) {
    if (node.second.isOk()) {
      active_nodes++;
    }
  }
  return active_nodes >= num_quorum;
}

NodeHealthChecker::NodeHealth::NodeHealth()
    : status{Status::Suspected},
      cv{std::make_unique<std::condition_variable_any>()} {};

void NodeHealthChecker::NodeHealth::Update(const std::int32_t cpr_status) {
  if (cpr::status::HTTP_OK == cpr_status) {
    status = Status::Healthy;
  } else if (Status::Healthy == status) {
    status = Status::Suspected;
  } else {
    status = Status::Unhealthy;
  }
}

bool NodeHealthChecker::NodeHealth::isOk() const {
  return Status::Healthy == status || Status::Suspected == status;
}

std::string NodeHealthChecker::NodeHealth::GetStatusStr() const {
  std::string status_str;
  switch (status) {
    case Status::Healthy:
      status_str = "Healthy";
      break;
    case Status::Suspected:
      status_str = "Suspected";
      break;
    case Status::Unhealthy:
      status_str = "Unhealthy";
      break;
    default:
      status_str = "Unhealthy";
      break;
  }
  return status_str;
}
