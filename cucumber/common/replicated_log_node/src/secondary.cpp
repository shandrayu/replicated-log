#include "replicated_log_node/secondary.h"

Secondary::Secondary(const std::string& host,
                                          const std::string& port)
    : m_host(host), m_port(port) {
  m_url = host + ":" + port;
}