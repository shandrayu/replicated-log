#ifndef REPLICATED_LOG_NODE_REPLICATED_LOG_MASTER_H__
#define REPLICATED_LOG_NODE_REPLICATED_LOG_MASTER_H__

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "countdown_latch/countdown_latch.h"
#include "replicated_log_node/node_health_checher.h"
#include "replicated_log_node/replicated_log_node.h"
#include "replicated_log_node/secondary.h"

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
  std::vector<Secondary> m_secondaries;
  NodeHealthChecker health_checker;

  bool m_retry{false};
  std::int32_t m_responce_timeout{1000};

  std::size_t m_message_id{0};
};

#endif  // REPLICATED_LOG_NODE_REPLICATED_LOG_MASTER_H__
