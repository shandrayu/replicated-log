#include <boost/algorithm/string.hpp>
#include <cpr/cpr.h>
#include <mif/common/log.h>

#include <iostream>
#include <string>
#include <vector>

#include "replicated_log_node/replicated_log_node.h"

// TODO: move definition to a separate file
class ReplicatedLogMaster : public ReplicatedLogNode {
 public:
  struct Secondary {
    std::string host;
    std::string port;
  };

  ReplicatedLogMaster() = default;
  virtual ~ReplicatedLogMaster() = default;

  void SetSecondaryNodesList(const std::string& secondary_nodes) {
    if (secondary_nodes.empty()) return;

    // TODO: Rethink splitting the string by delimiter
    std::vector<std::string> results;
    boost::split(results, secondary_nodes, [](char c) { return c == ','; });
    m_secondaries.clear();
    std::vector<std::string> secondary_splitted;
    for (const auto& result : results) {
      boost::split(secondary_splitted, result, [](char c) { return c == ':'; });
      m_secondaries.push_back(
          Secondary{secondary_splitted[0], secondary_splitted[1]});
    }
  }

 private:
  virtual void PostHandlerAdditionalFunctionality(
      int message_id, const std::string& message_body) override {
    Mif::Common::Unused(message_id);
    Mif::Common::Unused(message_body);
    auto json_message =
        m_messages[message_id].ToJson(message_id).toStyledString();
    for (const auto secondary : m_secondaries) {
      std::string url_string = secondary.host + ":" + secondary.port;
      cpr::Response r =
          cpr::Post(cpr::Url{url_string}, cpr::Body(json_message));
      MIF_LOG(Info) << "Post message to " << url_string;
      if (r.status_code == cpr::status::HTTP_OK) {
        MIF_LOG(Info) << "Done!";
      } else {
        MIF_LOG(Info) << "Post message confirmation is not received.";
        continue;
      }
    }
  }

  std::vector<Secondary> m_secondaries;
};

namespace {
namespace Detail {
namespace Config {
using SecondaryNodes = MIF_STATIC_STR("secondarynodes");
}  // namespace Config
}  // namespace Detail
}  // namespace

class LogApplication : public Mif::Application::HttpServer {
 public:
  using HttpServer::HttpServer;

  LogApplication(int argc, char const** argv)
      : Mif::Application::HttpServer(argc, argv),
        m_replicated_log(std::make_shared<ReplicatedLogMaster>()) {
    boost::program_options::options_description options{
        "Replicated Log Application, Master node options"};

    options.add_options()(
        Detail::Config::SecondaryNodes::Value,
        boost::program_options::value<std::string>(&m_secondaries)
            ->default_value(""),
        "List of secondary nodes in a format host1:port1;host2:port2 "
        "(0.0.0.0:55555,0.0.0.0:44444)");

    AddCustomOptions(options);
  }

 private:
  virtual void Init(Mif::Net::Http::ServerHandlers& handlers) override final {
    handlers.emplace(
        "/", std::bind(&ReplicatedLogMaster::RequestHandler, m_replicated_log,
                       std::placeholders::_1, std::placeholders::_2));
    m_replicated_log->SetSecondaryNodesList(m_secondaries);
  }

  std::shared_ptr<ReplicatedLogMaster> m_replicated_log;
  std::string m_secondaries;
};

int main(int argc, char const** argv) {
  return Mif::Application::Run<LogApplication>(argc, argv);
}
