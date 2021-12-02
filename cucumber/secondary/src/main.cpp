#include "replicated_log_node/replicated_log_secondary.h"

namespace {
namespace Detail {
namespace Config {
using ConstantDelay = MIF_STATIC_STR("responcedelay");
using ServerErrorMessageId = MIF_STATIC_STR("errormessageid");
}  // namespace Config
}  // namespace Detail
}  // namespace

class LogApplication : public Mif::Application::HttpServer {
 public:
  using HttpServer::HttpServer;

  LogApplication(int argc, char const** argv)
      : Mif::Application::HttpServer(argc, argv),
        m_replicated_log(std::make_shared<ReplicatedLogSecondary>()) {
    boost::program_options::options_description options{
        "Replicated Log Application, secondary node options"};

    options.add_options()(
        Detail::Config::ConstantDelay::Value,
        boost::program_options::value<std::size_t>(&m_response_delay)
            ->default_value(0),
        "Application responce delay, miliseconds")(
        Detail::Config::ServerErrorMessageId::Value,
        boost::program_options::value<int>(&m_server_error_message_id)
            ->default_value(-1),
        "Generate server error for message with this ID. Implemented by once"
        "skipping the message with requested ID, int");

    AddCustomOptions(options);
  }

 private:
  virtual void Init(Mif::Net::Http::ServerHandlers& handlers) override final {
    handlers.emplace("/", std::bind(&ReplicatedLogSecondary::RequestHandler,
                                    m_replicated_log, std::placeholders::_1,
                                    std::placeholders::_2));
    handlers.emplace(
        "/health",
        std::bind(&ReplicatedLogSecondary::HealthStatusRequestHandler,
                  m_replicated_log, std::placeholders::_1,
                  std::placeholders::_2));
    //  Pass application parameters to log node
    m_replicated_log->SetResponceDelay(m_response_delay);
    m_replicated_log->SetServerErrorMessageId(m_server_error_message_id);
  }

  std::shared_ptr<ReplicatedLogSecondary> m_replicated_log;
  std::size_t m_response_delay;
  int m_server_error_message_id{-1};
};

int main(int argc, char const** argv) {
  return Mif::Application::Run<LogApplication>(argc, argv);
}
