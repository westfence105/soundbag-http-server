#pragma once

#include <mutex>

#include "HttpServer.hpp"

namespace soundbag {
  class StatefulServer : public HttpServer {
      typedef HttpServer super;

      std::unordered_map<std::string, std::string> m_variables;
      std::mutex m_mutex;

    protected:
      virtual HttpResponse handleApi(const HttpRequest& request);

    public:
      StatefulServer(const std::string& host, int port, const std::string& configFileName = "server_config.json");
  };
}
