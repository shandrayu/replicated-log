// MIF
#include <mif/application/http_server.h>
#include <mif/common/log.h>
#include <mif/net/http/constants.h>

#include <functional>
#include <list>
#include <memory>
#include <mutex>

// TODO: move class to a separate file
class ReplicatedLogSecondary {
 public:
  using TMessageContainer = std::list<std::string>;

  void RequestHandler(Mif::Net::Http::IInputPack const& request,
                      Mif::Net::Http::IOutputPack& response) {
    auto const data = request.GetData();
    if (request.GetType() == Mif::Net::Http::Method::Type::Get) {
      MIF_LOG(Info) << "Get: Received bytes: " << data.size();
      response.SetCode(Mif::Net::Http::Code::Ok);
      auto data = ConvertMessagesToBuffer(m_messages);
      response.SetData(std::move(data));
    } else if (request.GetType() == Mif::Net::Http::Method::Type::Post) {
      MIF_LOG(Info) << "Post: Received bytes: " << data.size();
      std::string json_string(data.begin(), data.end());
      m_messages.emplace_back(json_string);
      response.SetCode(Mif::Net::Http::Code::Ok);
    }
  }

 private:
  std::vector<char> ConvertMessagesToBuffer(const TMessageContainer& messages) {
    std::vector<char> buffer;
    for (const auto& message : messages) {
      for (const auto& symbol : message) {
        buffer.push_back(symbol);
      }
    }
    return buffer;
  }

 private:
  TMessageContainer m_messages;
};

class Application : public Mif::Application::HttpServer {
 public:
  using HttpServer::HttpServer;

  Application(int argc, char const** argv)
      : Mif::Application::HttpServer(argc, argv),
        m_replicated_log(std::make_shared<ReplicatedLogSecondary>()) {}

 private:
  virtual void Init(Mif::Net::Http::ServerHandlers& handlers) override final {
    handlers.emplace("/", std::bind(&ReplicatedLogSecondary::RequestHandler,
                                    m_replicated_log, std::placeholders::_1,
                                    std::placeholders::_2));
  }

  std::shared_ptr<ReplicatedLogSecondary> m_replicated_log;
};

int main(int argc, char const** argv) {
  return Mif::Application::Run<Application>(argc, argv);
}
