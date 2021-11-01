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
  struct Message {
    Message() = default;
    Message(const std::string& string);
    Json::Value ToJson(int id) const;
    std::string data;
    Json::Value json;
  };

  ReplicatedLogNode();

  void RequestHandler(Mif::Net::Http::IInputPack const& request,
                      Mif::Net::Http::IOutputPack& response);

  virtual ~ReplicatedLogNode() = default;

 private:
  virtual void PostHandlerAdditionalFunctionality(
      int message_id, const std::string& message_body);

  void GetHandler(Mif::Net::Http::IInputPack const& request,
                  Mif::Net::Http::IOutputPack& response);
  void PostHandler(Mif::Net::Http::IInputPack const& request,
                   Mif::Net::Http::IOutputPack& response);

 protected:
  std::map<int, Message> m_messages;
  // TODO: Inplementation details
  std::unique_ptr<Json::CharReader> m_char_reader;
};

#endif  // REPLICATED_LOG_NODE_REPLICATED_LOG_NODE_H__
