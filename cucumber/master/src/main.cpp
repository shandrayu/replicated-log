#include <boost/algorithm/string.hpp>
#include <cpr/cpr.h>
#include <mif/common/log.h>

#include <chrono>
#include <future>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "replicated_log_node/replicated_log_node.h"

// TODO: move definition to a separate file
class ReplicatedLogMaster : public ReplicatedLogNode {
 public:
  struct Secondary {
    std::string host;
    std::string port;
  };

  struct NodeResponce {
    std::string url;
    bool is_received;
    std::future<cpr::Response> furute;
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

  void SetupWriteConcern(std::size_t write_concern,
                         std::size_t responce_timeout) {
    if (write_concern > 1 && write_concern < m_secondaries.size() + 1) {
      m_write_concern = write_concern;
    } else if (write_concern >= m_secondaries.size() + 1) {
      m_write_concern = m_secondaries.size() + 1;
      MIF_LOG(Info)
          << "Write concern cannot be more than number of available "
             "nodes + master node. Setting write concern to maximum level "
          << m_write_concern;
    }
    m_responce_timeout = responce_timeout;
  }

 private:
  virtual Mif::Net::Http::Code StoreMessage(
      int message_id, const std::string& message_body) override {
    const auto message = Message(message_body);

    auto id_position = m_messages.find(message_id);
    // TODO: change message_id meaning. It shall be a counter
    // increased internally in master not external data
    if (id_position == m_messages.end()) {
      m_messages[message_id] = Message(message_body);
      SendMessageToSecondaries(message_id, message);
    } else {
      // Nothing, message is present in database
    }
    return Mif::Net::Http::Code::Ok;
  }

  void SendMessageToSecondaries(int message_id, Message message) {
    // TODO: Readabiliry - move lambdas to class methods?
    auto SendMessages = [this](int message_id,
                               Message message) -> std::vector<NodeResponce> {
      auto json_message = message.ToJson(message_id).toStyledString();

      std::vector<NodeResponce> node_response;
      node_response.reserve(m_secondaries.size());

      for (const auto secondary : m_secondaries) {
        std::string url_string = secondary.host + ":" + secondary.port;
        std::promise<cpr::Response> p1;
        std::future<cpr::Response> f_completes = p1.get_future();
        std::thread(
            [](std::promise<cpr::Response> p1, const std::string& url_str,
               const std::string& mesage_str) {
              cpr::Response r =
                  cpr::Post(cpr::Url{url_str}, cpr::Body(mesage_str));
              p1.set_value_at_thread_exit(r);
            },
            std::move(p1), url_string, json_message)
            .detach();

        const bool kResponceReceived = false;
        node_response.emplace_back(NodeResponce{url_string, kResponceReceived,
                                                std::move(f_completes)});
      }
      return node_response;
    };

    auto GatherResponses =
        [this](std::vector<NodeResponce>& node_response,
               const std::chrono::system_clock::time_point& timeout)
        -> std::size_t {
      std::size_t current_concert_level = 1;
      for (auto& responce : node_response) {
        if (responce.is_received) {
          current_concert_level++;
        } else if (std::future_status::ready ==
                   responce.furute.wait_until(timeout)) {
          const auto http_response = responce.furute.get();
          auto status_code = http_response.status_code;
          if (cpr::status::HTTP_OK == status_code) {
            current_concert_level++;
          } else {
            MIF_LOG(Info) << "secondary node " << responce.url
                          << " post message is not successfull. Status code "
                          << status_code;
          }
        } else {
          MIF_LOG(Info) << "secondary node " << responce.url
                        << " post message confirmation is not received";
        }
      }
      return current_concert_level;
    };

    auto node_responce = SendMessages(message_id, message);

    std::chrono::milliseconds retry_delay;
    std::chrono::system_clock::time_point timeout;
    do {
      retry_delay = std::chrono::milliseconds(m_responce_timeout);
      timeout = std::chrono::system_clock::now() + retry_delay;
    } while (m_write_concern > GatherResponses(node_responce, timeout));
  }

  std::vector<Secondary> m_secondaries;
  std::size_t m_write_concern{1};
  std::size_t m_responce_timeout{1000};
};

namespace {
namespace Detail {
namespace Config {
using SecondaryNodes = MIF_STATIC_STR("secondarynodes");
using WriteConcernLevel = MIF_STATIC_STR("writeconcernlevel");
using ResponseTimeout = MIF_STATIC_STR("responsetimeout");
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
        Detail::Config::WriteConcernLevel::Value,
        boost::program_options::value<std::size_t>(&m_write_concern_level)
            ->default_value(1),
        "Write concern level. Perform append only if the amount of write "
        "confirmations received before the timeout is bigger than write "
        "concern level")(Detail::Config::ResponseTimeout::Value,
                         boost::program_options::value<std::size_t>(
                             &m_response_timeout_ms)
                             ->default_value(1000),
                         "Secondary node response timeout, ms");

    AddCustomOptions(options);
  }

 private:
  virtual void Init(Mif::Net::Http::ServerHandlers& handlers) override final {
    handlers.emplace(
        "/", std::bind(&ReplicatedLogMaster::RequestHandler, m_replicated_log,
                       std::placeholders::_1, std::placeholders::_2));
    m_replicated_log->SetSecondaryNodesList(m_secondaries);
    m_replicated_log->SetupWriteConcern(m_write_concern_level,
                                        m_response_timeout_ms);
  }

  std::shared_ptr<ReplicatedLogMaster> m_replicated_log;
  std::string m_secondaries;
  std::size_t m_write_concern_level;
  std::size_t m_response_timeout_ms;
};

int main(int argc, char const** argv) {
  return Mif::Application::Run<LogApplication>(argc, argv);
}
