#ifndef REPLICATED_LOG_NODE_REPLICATED_LOG_SECONDARY_H__
#define REPLICATED_LOG_NODE_REPLICATED_LOG_SECONDARY_H__

#include "replicated_log_node/replicated_log_node.h"

class ReplicatedLogSecondary : public ReplicatedLogNode {
 public:
  ReplicatedLogSecondary() = default;
  virtual ~ReplicatedLogSecondary() = default;

  void SetResponceDelay(const std::size_t& delay);

 private:
  virtual Mif::Net::Http::Code StoreMessage(const Json::Value& node) override;

  std::size_t m_response_delay_ms{0};
};

#endif  // REPLICATED_LOG_NODE_REPLICATED_LOG_SECONDARY_H__
