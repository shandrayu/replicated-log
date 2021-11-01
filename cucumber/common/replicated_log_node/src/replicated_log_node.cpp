#include "replicated_log_node/replicated_log_node.h"

namespace {
namespace detail {
// std::string json_node_to_string(const Json::Value& node) {
//   Json::StreamWriterBuilder builder;
//   return Json::writeString(builder, node);
// }

std::vector<char> convert_messages_to_buffer(
    const std::map<int, ReplicatedLogNode::Message>& messages) {
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
}  // namespace detail
}

ReplicatedLogNode::Message::Message(const std::string& string) : data(string) {}

Json::Value ReplicatedLogNode::Message::ToJson(int id) const {
  Json::Value node;
  node["id"] = id;
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

void ReplicatedLogNode::PostHandlerAdditionalFunctionality(
    int message_id, const std::string& message_body) {
  Mif::Common::Unused(message_id);
  Mif::Common::Unused(message_body);
}

void ReplicatedLogNode::GetHandler(Mif::Net::Http::IInputPack const& request,
                                   Mif::Net::Http::IOutputPack& response) {
  auto const data = request.GetData();
  MIF_LOG(Info) << "Get: Received bytes: " << data.size()
                << ". Start processing...";

  auto converted_messages = detail::convert_messages_to_buffer(m_messages);
  response.SetData(std::move(converted_messages));

  MIF_LOG(Info) << "Get: Done!";
  response.SetCode(Mif::Net::Http::Code::Ok);
}

void ReplicatedLogNode::PostHandler(Mif::Net::Http::IInputPack const& request,
                                    Mif::Net::Http::IOutputPack& response) {
  auto const data = request.GetData();
  MIF_LOG(Info) << "Post: Received bytes: " << data.size()
                << ". Start processing...";
  ;

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
  const auto message_body = root["message"].asString();
  auto id_position = m_messages.find(id);
  if (id_position == m_messages.end()) {
    m_messages[id] = Message(message_body);
  } else {
    // Nothing, message is present in database
  }

  PostHandlerAdditionalFunctionality(id, message_body);

  MIF_LOG(Info) << "Post: OK";
  response.SetCode(Mif::Net::Http::Code::Ok);
}
