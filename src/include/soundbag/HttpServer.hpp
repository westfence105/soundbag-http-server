#pragma once

#include <unordered_map>
#include <list>
#include <future>
#include <atomic>

#include <nlohmann/json.hpp>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

static constexpr int BUFSIZE = 1024;

namespace soundbag {
  class HttpServer {
    std::string m_host;
    int m_port;

    std::future<void> m_mainThread;
    std::list<std::future<void>> m_threads;
    std::atomic<bool> m_loop;

    nlohmann::json m_config;

    void serverMain();
    void handleConnection(int destSock);

    protected:
      inline const nlohmann::json& config() const { return m_config;}

      HttpResponse handlePage(const std::string& path);
      HttpResponse handleMedia(const std::string& path);
      std::string  findMediaLibrary(const std::string& key) const;

      virtual HttpResponse handleRequest(const HttpRequest& request);
      inline virtual HttpResponse handleApi(const HttpRequest& request) {
        return HttpResponse(404);
      }

    public:
      HttpServer(const std::string& host, int port, const std::string& configFileName = "server_config.json");

      void start();
      void stop();
  };
}
