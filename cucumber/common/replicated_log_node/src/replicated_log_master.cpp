#include "replicated_log_node/replicated_log_master.h"

#include <boost/algorithm/string.hpp>
#include <cpr/cpr.h>
#include <mif/common/log.h>

ReplicatedLogMaster::ReplicatedLogMaster() {}

ReplicatedLogMaster::~ReplicatedLogMaster() {}

void ReplicatedLogMaster::SetSecondaryNodesList(
    const std::string& secondary_nodes) {
  if (secondary_nodes.empty()) return;

  // TODO: Rethink splitting the string by delimiter
  std::vector<std::string> results;
  boost::split(results, secondary_nodes, [](char c) { return c == ','; });
  health_checker.Reset();
  m_secondaries.clear();
  std::vector<std::string> secondary_splitted;
  for (const auto& result : results) {
    boost::split(secondary_splitted, result, [](char c) { return c == ':'; });
    auto secondary = Secondary{secondary_splitted[0], secondary_splitted[1]};
    m_secondaries.emplace_back(secondary);
  }
  health_checker.Setup(m_secondaries);
}

void ReplicatedLogMaster::SetHealthCheckPeriod(const std::size_t period_ms) {
  health_checker.SetHealthCheckPeriod(period_ms);
}

void ReplicatedLogMaster::EnableRetry(bool enable) { m_retry = enable; }

Mif::Net::Http::Code ReplicatedLogMaster::StoreMessage(
    const Json::Value& node) {
  if (!health_checker.HasQuorum()) {
    return Mif::Net::Http::Code::Unavaliable;
  }

  const auto message_body = node["message"].asString();
  const int write_concern = node["write_concern"].asInt();
  InternalMessage message;
  {
    std::lock_guard<std::mutex> lck(m_message_queue_mutex);
    message = InternalMessage(m_message_id, message_body);
    m_messages[m_message_id++] = message;
  }

  SendMessageToSecondaries(message, write_concern);
  return Mif::Net::Http::Code::Ok;
}

void ReplicatedLogMaster::SendMessageToSecondaries(InternalMessage message,
                                                   int write_concern) {
  auto json_message = message.ToJson().toStyledString();

  auto countdown = std::make_shared<CountDownLatch>(write_concern - 1);
  for (const auto& secondary : m_secondaries) {
    // TODO: create thread only if the node is healthy
    // TODO: capture this. It will fail if program will be closed before the
    // thread has finished
    // TODO: join all threads in destructor
    std::thread(
        [this](std::shared_ptr<CountDownLatch> countdown,
               const std::string& url_str, const std::string& message_str,
               const int message_id, bool retry_if_failed) {
          cpr::Response responce;
          do {
            health_checker.WaitOkStatus(url_str);

            MIF_LOG(Info) << "Try to send message id=" << message_id << " to "
                          << url_str;
            responce = cpr::Post(cpr::Url{url_str}, cpr::Body(message_str),
                                 cpr::Timeout{m_responce_timeout});

          } while (cpr::status::HTTP_OK != responce.status_code &&
                   retry_if_failed);
          if (cpr::status::HTTP_OK == responce.status_code) {
            MIF_LOG(Info) << "Message id=" << message_id << " to " << url_str
                          << " sent!";
            countdown->count_down();
          } else {
            // TODO: not possible in this implementation
            MIF_LOG(Info) << "Message id=" << message_id << " to " << url_str
                          << " is not sent!";
          }
        },
        countdown, secondary.GetUrl(), json_message, message.id, m_retry)
        .detach();
  }
  // TODO: Implement wait with timeout?
  countdown->await(/*timeout*/);
}
