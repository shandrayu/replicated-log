#ifndef REPLICATED_LOG_NODE_REPLICATED_LOG_SECONDARY_H__
#define REPLICATED_LOG_NODE_REPLICATED_LOG_SECONDARY_H__

#include "replicated_log_node/replicated_log_node.h"

class ReplicatedLogSecondary : public ReplicatedLogNode {
 public:
  ReplicatedLogSecondary() = default;
  virtual ~ReplicatedLogSecondary() = default;

  void HealthStatusRequestHandler(Mif::Net::Http::IInputPack const& request,
                                  Mif::Net::Http::IOutputPack& response);

  void SetResponceDelay(const std::size_t delay);
  void SetServerErrorMessageId(const int id);

 private:
  virtual Mif::Net::Http::Code StoreMessage(const Json::Value& node) override;

  struct ServerError {
    int message_id{-1};
    bool triggered{false};
  };

  std::size_t m_response_delay_ms{0};
  ServerError m_server_error;
};

#endif  // REPLICATED_LOG_NODE_REPLICATED_LOG_SECONDARY_H__
