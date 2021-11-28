#ifndef REPLICATED_LOG_NODE_REPLICATED_LOG_MASTER_H__
#define REPLICATED_LOG_NODE_REPLICATED_LOG_MASTER_H__

#include <boost/algorithm/string.hpp>
#include <cpr/cpr.h>
#include <mif/common/log.h>

#include <chrono>
#include <future>
#include <string>
#include <utility>
#include <vector>

#include "countdown_latch/countdown_latch.h"
#include "replicated_log_node/replicated_log_node.h"

class ReplicatedLogMaster : public ReplicatedLogNode {
 public:
  struct Secondary {
    std::string host;
    std::string port;
  };

  struct NodeResponce {
    std::string url;
    cpr::Response responce;
  };

  ReplicatedLogMaster() = default;
  virtual ~ReplicatedLogMaster() = default;

  void SetSecondaryNodesList(const std::string& secondary_nodes);
  void EnableRetry(bool enable);

 private:
  virtual Mif::Net::Http::Code StoreMessage(const Json::Value& node) override;
  void SendMessageToSecondaries(InternalMessage message, int write_concern);

  std::vector<Secondary> m_secondaries;
  bool m_retry{false};
  std::size_t m_responce_timeout{1000};

  std::size_t m_message_id{0};
};

#endif  // REPLICATED_LOG_NODE_REPLICATED_LOG_MASTER_H__
