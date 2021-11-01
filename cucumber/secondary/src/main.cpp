// MIF
#include <json/json.h>
#include <mif/application/http_server.h>
#include <mif/common/log.h>
#include <mif/net/http/constants.h>

#include <chrono>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

namespace utils {
std::string json_node_to_string(const Json::Value& node) {
  Json::StreamWriterBuilder builder;
  return Json::writeString(builder, node);
}
}  // namespace utils

// TODO: move class to a separate file
class ReplicatedLogSecondary {
 public:
  struct Message {
    Message() = default;
    Message(const std::string& string) : data(string) {}
    std::string data;
    Json::Value json;
    Json::Value ToJson(int id) const {
      Json::Value node;
      node["id"] = id;
      node["message"] = data;
      return node;
    }
  };

  using TMessageContainer = std::map<int, Message>;

  ReplicatedLogSecondary() {
    Json::CharReaderBuilder builder;
    m_char_reader = std::unique_ptr<Json::CharReader>(builder.newCharReader());
  }

  void SetResponceDelay(const std::size_t& delay) {
    m_response_delay_ms = delay;
  }

  void RequestHandler(Mif::Net::Http::IInputPack const& request,
                      Mif::Net::Http::IOutputPack& response) {
    auto const data = request.GetData();
    if (request.GetType() == Mif::Net::Http::Method::Type::Get) {
      MIF_LOG(Info) << "Get: Received bytes: " << data.size() << ". Start processing...";

      auto converted_messages = ConvertMessagesToBuffer(m_messages);
      response.SetData(std::move(converted_messages));

      MIF_LOG(Info) << "Get: Done!";
      response.SetCode(Mif::Net::Http::Code::Ok);
    } else if (request.GetType() == Mif::Net::Http::Method::Type::Post) {
      MIF_LOG(Info) << "Post: Received bytes: " << data.size() << ". Start processing...";;

      Json::Value root;
      std::string errors;
      bool parsing_successful = m_char_reader->parse(
          data.data(), data.data() + data.size(), &root, &errors);
      if (!parsing_successful) {
        MIF_LOG(Info) << errors;
        response.SetCode(Mif::Net::Http::Code::BadRequest);
        return;
      }

      const auto id = root["id"].asInt();
      auto id_position = m_messages.find(id);
      if (id_position == m_messages.end()) {
        m_messages[id] = Message(root["message"].asString());
      } else {
        // Nothing, message is present in database
      }

      std::this_thread::sleep_for(
          std::chrono::milliseconds(m_response_delay_ms));

      MIF_LOG(Info) << "Post: OK";
      response.SetCode(Mif::Net::Http::Code::Ok);
    }
  }

 private:
  std::vector<char> ConvertMessagesToBuffer(const TMessageContainer& messages) {
    Json::Value message_array(Json::arrayValue);
    for (const auto& message : messages) {
      MIF_LOG(Info) << message.first << ": " << message.second.data;
      message_array.append(message.second.ToJson(message.first));
    }

    std::vector<char> buffer;
    const std::string messages_str = message_array.toStyledString();
    for (const auto& symbol : messages_str) {
      buffer.push_back(symbol);
    }
    return buffer;
  }

 private:
  TMessageContainer m_messages;
  std::unique_ptr<Json::CharReader> m_char_reader;
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
