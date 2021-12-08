#ifndef REPLICATED_LOG_NODE_SECONDARY_H__
#define REPLICATED_LOG_NODE_SECONDARY_H__

#include <string>

class Secondary {
 public:
  Secondary(const std::string& host, const std::string& port);
  std::string GetUrl() const { return m_url; }
  std::string GetHash() const { return GetUrl(); }

 private:
  std::string m_host;
  std::string m_port;
  std::string m_url;
};

#endif  // REPLICATED_LOG_NODE_SECONDARY_H__
