#ifndef REPLICATED_LOG_NODE_NODE_HEALTH_CHECHER_H__
#define REPLICATED_LOG_NODE_NODE_HEALTH_CHECHER_H__

#include <atomic>
#include <condition_variable>
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

#include "replicated_log_node/secondary.h"

class NodeHealthChecker {
 public:
  enum class Status : int { Healthy = 0, Suspected = 1, Unhealthy = 2 };

  NodeHealthChecker() = default;
  ~NodeHealthChecker();

  NodeHealthChecker(NodeHealthChecker&&) = default;
  NodeHealthChecker& operator=(NodeHealthChecker&&) = default;
  NodeHealthChecker(const NodeHealthChecker& other) = delete;
  NodeHealthChecker& operator=(const NodeHealthChecker&) = delete;

  void Setup(const std::vector<Secondary>& secondaries);
  void Reset();

  void SetHealthCheckPeriod(const std::size_t period_ms);
  void WaitOkStatus(const std::string secondary_hash);
  bool HasQuorum() const;

 private:
  struct NodeHealth {
    NodeHealth();
    NodeHealth(NodeHealth&&) = default;
    NodeHealth& operator=(NodeHealth&&) = default;
    NodeHealth(const NodeHealth& other) = delete;
    NodeHealth& operator=(const NodeHealth&) = delete;

    void Update(const std::int32_t cpr_status);
    bool isOk() const;
    std::string GetStatusStr() const;

    Status status;
    std::unique_ptr<std::condition_variable_any> cv;
    std::thread thread;
  };

  std::size_t m_health_check_period_ms{1000};

  std::map<std::string, NodeHealth> m_health_status;
  std::atomic_bool m_terminate_health_status_check{false};
  mutable std::shared_mutex m_health_status_mutex;
};

#endif // REPLICATED_LOG_NODE_NODE_HEALTH_CHECHER_H__
