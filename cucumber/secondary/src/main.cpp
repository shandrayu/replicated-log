#include "replicated_log_node/replicated_log_node.h"

class ReplicatedLogSecondary : public ReplicatedLogNode {
 public:
  ReplicatedLogSecondary() = default;
  virtual ~ReplicatedLogSecondary() = default;

  void SetResponceDelay(const std::size_t& delay) {
    m_response_delay_ms = delay;
  }

 private:
  virtual void PostHandlerAdditionalFunctionality(
      int message_id, const std::string& message_body) override {
    Mif::Common::Unused(message_id);
    Mif::Common::Unused(message_body);
    std::this_thread::sleep_for(std::chrono::milliseconds(m_response_delay_ms));
  }

  std::size_t m_response_delay_ms{0};
};

namespace {
namespace Detail {
namespace Config {
using ConstantDelay = MIF_STATIC_STR("responcedelay");
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
        "Log Application options"};

    options.add_options()(
        Detail::Config::ConstantDelay::Value,
        boost::program_options::value<std::size_t>(&m_response_delay)
            ->default_value(0),
        "Application responce delay, miliseconds");

    AddCustomOptions(options);
  }

 private:
  virtual void Init(Mif::Net::Http::ServerHandlers& handlers) override final {
    handlers.emplace("/", std::bind(&ReplicatedLogSecondary::RequestHandler,
                                    m_replicated_log, std::placeholders::_1,
                                    std::placeholders::_2));
    m_replicated_log->SetResponceDelay(m_response_delay);
  }

  std::shared_ptr<ReplicatedLogSecondary> m_replicated_log;
  std::size_t m_response_delay;
};

int main(int argc, char const** argv) {
  return Mif::Application::Run<LogApplication>(argc, argv);
}
