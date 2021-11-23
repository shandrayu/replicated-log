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

#include "replicated_log_node/replicated_log_node.h"

class ReplicatedLogMaster : public ReplicatedLogNode {
 public:
  struct Secondary {
    std::string host;
    std::string port;
  };

  struct NodeResponce {
    std::string url;
    bool is_received;
    std::future<cpr::Response> furute;
  };

  ReplicatedLogMaster() = default;
  virtual ~ReplicatedLogMaster() = default;

  void SetSecondaryNodesList(const std::string& secondary_nodes);

 private:
  virtual Mif::Net::Http::Code StoreMessage(const Json::Value& node) override;
  void SendMessageToSecondaries(InternalMessage message,
                                std::size_t write_concern);
  std::vector<NodeResponce> SendMessages(InternalMessage message);
  std::size_t GatherResponses(
      std::vector<NodeResponce>& node_response,
      const std::chrono::system_clock::time_point& timeout);
      
  std::vector<Secondary> m_secondaries;
  std::size_t m_responce_timeout{1000};
  std::size_t m_message_id{0};
};

#endif  // REPLICATED_LOG_NODE_REPLICATED_LOG_MASTER_H__
