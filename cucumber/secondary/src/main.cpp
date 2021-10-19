// MIF
#include <json/json.h>
#include <mif/application/http_server.h>
#include <mif/common/log.h>
#include <mif/net/http/constants.h>

#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <mutex>

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

  void RequestHandler(Mif::Net::Http::IInputPack const& request,
                      Mif::Net::Http::IOutputPack& response) {
    auto const data = request.GetData();
    if (request.GetType() == Mif::Net::Http::Method::Type::Get) {
      MIF_LOG(Info) << "Get: Received bytes: " << data.size();

      auto converted_messages = ConvertMessagesToBuffer(m_messages);
      response.SetData(std::move(converted_messages));
      response.SetCode(Mif::Net::Http::Code::Ok);
    } else if (request.GetType() == Mif::Net::Http::Method::Type::Post) {
      MIF_LOG(Info) << "Post: Received bytes: " << data.size();

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
