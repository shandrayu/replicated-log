#ifndef REPLICATED_LOG_NODE_REPLICATED_LOG_MASTER_H__
#define REPLICATED_LOG_NODE_REPLICATED_LOG_MASTER_H__

#include <boost/algorithm/string.hpp>
#include <cpr/cpr.h>
#include <mif/common/log.h>

#include <chrono>
#include <condition_variable>
#include <future>
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>
#include <utility>
#include <vector>

#include "countdown_latch/countdown_latch.h"
#include "replicated_log_node/replicated_log_node.h"

class ReplicatedLogMaster : public ReplicatedLogNode {
 public:
  ReplicatedLogMaster();
  virtual ~ReplicatedLogMaster();

  void SetSecondaryNodesList(const std::string& secondary_nodes);
  void SetHealthCheckPeriod(const std::size_t period);
  void EnableRetry(bool enable);

 private:
  virtual Mif::Net::Http::Code StoreMessage(const Json::Value& node) override;
  void SendMessageToSecondaries(InternalMessage message, int write_concern);

  class Secondary {
   public:
    Secondary(const std::string& host, const std::string& port);
    std::string GetUrl() const { return m_url; }
    std::string GetHash() const { return GetUrl(); }

   private:
    std::string m_host;
    std::string m_port;
    std::string m_url;
  };

  struct NodeHealth {
    enum class Status : int { Healthy = 0, Suspected = 1, Unhealthy = 2 };
    // Status is a three-state state machine
    void Update(const std::int32_t cpr_status);
    // TODO: public data members
    Status status;
    // TODO: probably ill-formed implementation
    // The intertion is to have a separate notification mechanism for every
    // message. Maybe there is a bette way to do it.
    std::unique_ptr<std::condition_variable_any> cv;

    NodeHealth()
        : status{Status::Suspected},
          cv{std::make_unique<std::condition_variable_any>()} {};
    ~NodeHealth() = default;
    // Movable
    NodeHealth(NodeHealth&&) = default;
    NodeHealth& operator=(NodeHealth&&) = default;
    // Non-copyable
    NodeHealth(const NodeHealth& other) = delete;
    NodeHealth& operator=(const NodeHealth&) = delete;
  };

  std::string GetStatusStr(NodeHealth::Status status) const;

  mutable std::shared_mutex m_secondary_status_mutex;
  std::vector<Secondary> m_secondaries;
  std::map<std::string, NodeHealth> m_secondary_health;
  std::size_t m_health_check_period_ms{1000};
  std::atomic_bool m_terminate_health_status_check{false};
  std::thread m_health_check_thread;

  bool m_retry{false};
  std::int32_t m_responce_timeout{1000};

  std::size_t m_message_id{0};
};

#endif  // REPLICATED_LOG_NODE_REPLICATED_LOG_MASTER_H__
