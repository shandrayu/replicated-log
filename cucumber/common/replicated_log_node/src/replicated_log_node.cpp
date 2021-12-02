#include "replicated_log_node/replicated_log_node.h"

namespace {
namespace detail {
std::vector<char> convert_consecutive_messages_to_buffer(
    const std::map<int, ReplicatedLogNode::InternalMessage>& messages) {
  Json::Value message_array(Json::arrayValue);
  int previous_id = -1;
  for (const auto& message : messages) {
    if (message.first != previous_id + 1) {
      // The message has ID bigger than expected, do not show it
      break;
    }
    message_array.append(message.second.ToJsonDataOnly());
    previous_id = message.first;
  }

  std::vector<char> buffer;
  const std::string messages_str = message_array.toStyledString();
  for (const auto& symbol : messages_str) {
    buffer.push_back(symbol);
  }
  return buffer;
}
}  // namespace detail
}  // namespace

ReplicatedLogNode::InternalMessage::InternalMessage(int id,
                                                    const std::string& string)
    : id(id), data(string) {}

Json::Value ReplicatedLogNode::InternalMessage::ToJson() const {
  Json::Value node;
  node["id"] = id;
  node["message"] = data;
  return node;
}

Json::Value ReplicatedLogNode::InternalMessage::ToJsonDataOnly() const {
  Json::Value node;
  node["message"] = data;
  return node;
}

ReplicatedLogNode::ReplicatedLogNode() {
  Json::CharReaderBuilder builder;
  m_char_reader = std::unique_ptr<Json::CharReader>(builder.newCharReader());
}

void ReplicatedLogNode::RequestHandler(
    Mif::Net::Http::IInputPack const& request,
    Mif::Net::Http::IOutputPack& response) {
  if (request.GetType() == Mif::Net::Http::Method::Type::Get) {
    GetHandler(request, response);
  } else if (request.GetType() == Mif::Net::Http::Method::Type::Post) {
    PostHandler(request, response);
  }
}

Mif::Net::Http::Code ReplicatedLogNode::StoreMessage(const Json::Value& node) {
  Mif::Common::Unused(node);
  return Mif::Net::Http::Code::NotImplemented;
}

void ReplicatedLogNode::GetHandler(Mif::Net::Http::IInputPack const& request,
                                   Mif::Net::Http::IOutputPack& response) {
  auto const data = request.GetData();
  MIF_LOG(Info) << "Get: Received bytes: " << data.size()
                << ". Start processing...";
  {
    std::lock_guard<std::mutex> lck(m_message_queue_mutex);
    auto converted_messages =
        detail::convert_consecutive_messages_to_buffer(m_messages);
    // TODO: move out the lock? Left here for readability
    response.SetData(std::move(converted_messages));
  }

  MIF_LOG(Info) << "Get: Done!";
  response.SetCode(Mif::Net::Http::Code::Ok);
}

void ReplicatedLogNode::PostHandler(Mif::Net::Http::IInputPack const& request,
                                    Mif::Net::Http::IOutputPack& response) {
  auto const data = request.GetData();
  MIF_LOG(Info) << "Post: Received bytes: " << data.size()
                << ". Start processing...";

  Json::Value root;
  std::string errors;
  bool parsing_successful = m_char_reader->parse(
      data.data(), data.data() + data.size(), &root, &errors);
  if (!parsing_successful) {
    MIF_LOG(Info) << "Error in message parsing: " << errors;
    response.SetCode(Mif::Net::Http::Code::BadRequest);
    return;
  }

  const auto status = StoreMessage(root);

  MIF_LOG(Info) << "Post: Done! Status "
                << ((Mif::Net::Http::Code::Ok == status) ? "OK" : "not OK :)");
  response.SetCode(status);
}
