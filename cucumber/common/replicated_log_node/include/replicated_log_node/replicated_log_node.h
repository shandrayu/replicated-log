#ifndef REPLICATED_LOG_NODE_REPLICATED_LOG_NODE_H__
#define REPLICATED_LOG_NODE_REPLICATED_LOG_NODE_H__

// MIF
#include <json/json.h>
#include <mif/application/http_server.h>
#include <mif/common/log.h>
#include <mif/common/unused.h>
#include <mif/net/http/constants.h>

#include <chrono>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

// TODO: Add namespace
// TODO: create a library or a makefile
class ReplicatedLogNode {
 public:
  // TODO: public data fields, struct instead of class...
  struct InternalMessage {
    InternalMessage() = default;
    InternalMessage(int id, const std::string& string);
    Json::Value ToJson() const;
    Json::Value ToJsonDataOnly() const;
    int id;
    std::string data;
  };

  ReplicatedLogNode();

  void RequestHandler(Mif::Net::Http::IInputPack const& request,
                      Mif::Net::Http::IOutputPack& response);

  virtual ~ReplicatedLogNode() = default;

 private:
  virtual Mif::Net::Http::Code StoreMessage(const Json::Value& node);

  void GetHandler(Mif::Net::Http::IInputPack const& request,
                  Mif::Net::Http::IOutputPack& response);
  void PostHandler(Mif::Net::Http::IInputPack const& request,
                   Mif::Net::Http::IOutputPack& response);

 protected:
  std::mutex m_message_queue_mutex;
  std::map<int, InternalMessage> m_messages;
};

#endif  // REPLICATED_LOG_NODE_REPLICATED_LOG_NODE_H__
