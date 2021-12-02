#include <mif/application/http_server.h>

#include <string>

#include "replicated_log_node/replicated_log_master.h"

namespace {
namespace Detail {
namespace Config {
using SecondaryNodes = MIF_STATIC_STR("secondarynodes");
using Retry = MIF_STATIC_STR("retry");
using HealthCheckPeriod = MIF_STATIC_STR("healthcheckperiod");
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
        "(0.0.0.0:55555,0.0.0.0:44444)")(
        Detail::Config::Retry::Value,
        boost::program_options::bool_switch(&m_retry)->default_value(false),
        "Enable retry functionality")(
        Detail::Config::HealthCheckPeriod::Value,
        boost::program_options::value<std::size_t>(&m_health_check_period_ms)
            ->default_value(3000),
        "Period to check the health status from secondary nodes");

    AddCustomOptions(options);
  }

 private:
  virtual void Init(Mif::Net::Http::ServerHandlers& handlers) override final {
    m_replicated_log->SetSecondaryNodesList(m_secondaries);
    m_replicated_log->EnableRetry(m_retry);
    m_replicated_log->SetHealthCheckPeriod(m_health_check_period_ms);
    handlers.emplace(
        "/", std::bind(&ReplicatedLogMaster::RequestHandler, m_replicated_log,
                       std::placeholders::_1, std::placeholders::_2));
  }

  std::shared_ptr<ReplicatedLogMaster> m_replicated_log;
  std::string m_secondaries;
  std::size_t m_health_check_period_ms;
  bool m_retry;
};

int main(int argc, char const** argv) {
  return Mif::Application::Run<LogApplication>(argc, argv);
}
