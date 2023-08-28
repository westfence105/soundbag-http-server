#include <iostream>
#include <sstream>
#include <vector>
#include <thread>
#include <functional>
#include <filesystem>

#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#include "soundbag/HttpServer.hpp"

using namespace std::literals::string_literals;
using namespace std::chrono_literals;
using json = nlohmann::json;

using namespace soundbag;

HttpServer::HttpServer(const std::string& host, int port, const std::string& configFileName) {
  m_host = host;
  m_port = port;
  m_loop.store(true);

  std::ifstream configFile(configFileName);
  if (configFile) {
    m_config = json::parse(configFile);
  }
}

void HttpServer::start() {
  m_mainThread = std::async(std::launch::async, std::bind(&HttpServer::serverMain, this));
  onStart();
}

void HttpServer::stop() {
  onStop();
  m_loop.store(false);
  m_mainThread.get();
}

void HttpServer::serverMain() {
  int status;
  int srcSock, destSock;
  struct sockaddr_in srcAddr, destAddr;
  socklen_t destAddrSize;

  srcAddr.sin_port = htons(m_port);
  srcAddr.sin_family = AF_INET;
  srcAddr.sin_addr.s_addr = m_host.size() ? inet_addr(m_host.c_str()) : htonl(INADDR_ANY);

  srcSock = socket(AF_INET, SOCK_STREAM, 0);
  struct timeval timeout;
  timeout.tv_sec = 2;
  timeout.tv_usec = 0;
  status = setsockopt(srcSock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
  for (int i = 0; i < 3; ++i) {
    if ((status = bind(srcSock, (struct sockaddr*)&srcAddr, sizeof(srcAddr))) == 0) {
      break;
    }
  }
  if (status) {
    std::cerr << "Error while binding socket." << std::endl;
    return;
  }

  int queueSize = 100;
  if (m_config.is_object() && m_config.count("queueSize") && m_config["queueSize"].is_number()) {
    queueSize = m_config["queueSize"];
  }
  listen(srcSock, queueSize);
  while (m_loop.load()) {
    destSock = accept(srcSock, (struct sockaddr*)&destAddr, &destAddrSize);
    if (destSock >= 0) {
      m_threads.push_back(std::async(std::launch::async, std::bind(&HttpServer::handleConnection, this, destSock)));
    }
  }

  for (auto& t : m_threads) {
    if (t.wait_for(0ms) != std::future_status::ready) {
      std::cout << "Waiting connection closed..." << std::endl;
      t.wait();
    }
    t.get();
  }

  close(srcSock);
}

void HttpServer::handleConnection(int destSock) {
  size_t recvSize;
  char buf[BUFSIZE+1];
  std::ostringstream oss;
  do {
    memset(buf, 0, BUFSIZE+1);
    recvSize = recv(destSock, buf, BUFSIZE, 0);
    oss << buf;
  } while (recvSize >= BUFSIZE);

  HttpRequest request(oss.str());
  HttpResponse response = handleRequest(request);

  std::string responseStr = response.serialize();
  send(destSock, responseStr.c_str(), responseStr.length(), 0);

  close(destSock);
}

HttpResponse HttpServer::handleRequest(const HttpRequest& request) {
  std::string path = request.path();
  if (path.starts_with("/api/")) {
    return handleApi(request);
  }
  else if (path.starts_with("/media/")) {
    return handleMedia(path.substr(7));
  }
  else {
    return handlePage(path.substr(1));
  }
}

HttpResponse HttpServer::handlePage(const std::string& path) {
  std::string filePath;
  std::string ext = "html";
  size_t extPos = path.find_first_of('.');
  if (extPos == std::string::npos) {
    if (path.find_first_of('/') == std::string::npos) {
      filePath = "pages/"s + path + "/index.html";
    }
    else {
      filePath = "pages/"s + path + ".html";
    }
  }
  else if (extPos+1 < path.length()) {
    ext = path.substr(extPos+1);
    filePath = "pages/"s + path;
  }

  std::ifstream file(filePath, std::ios::in);
  if (file) {
    return HttpResponse(file, ext);
  }
  else {
    return HttpResponse(404);
  }
}

std::string HttpServer::findMediaLibrary(const std::string& key) const {
  if (m_config.is_object() && m_config.count("media") && m_config["media"].is_object() && m_config["media"].count(key)) {
    return m_config["media"][key];
  }
  else {
    return "";
  }
}

HttpResponse HttpServer::handleMedia(const std::string& path) {
  std::vector<std::string> parsedPath = splitPath(path);
  std::string library;
  if (parsedPath.size() >= 2 && (library = findMediaLibrary(parsedPath[0])) != "") {
    parsedPath[0] = library;

    std::string ext;
    std::string& filename = parsedPath[parsedPath.size()-1];
    size_t p;
    size_t offset = 0;

    // Decode url
    bool decoded = false;
    while (offset < filename.size() && (p = filename.find_first_of('%', offset)) != std::string::npos) {
      if (p < filename.size() - 2) {
        std::string codeStr = filename.substr(p + 1, 2);
        try {
          unsigned int code = std::stoul(codeStr, NULL, 16);
          filename.replace(p, 3, 1, (char)code);
          offset = p + 1;
        }
        catch (std::invalid_argument e) {
          std::cerr << e.what() << std::endl;
          offset = p + 3;
          continue;
        }
      }
    }

    // Find extension
    p = filename.find_first_of('.');
    if (p != std::string::npos && p < filename.size() - 1) {
      ext = filename.substr(p+1);
    }

    // Load file
    std::ifstream file(joinPath(parsedPath), std::ios::in|std::ios::binary);
    if (file) {
      return HttpResponse(file, ext);
    }
  }

  return HttpResponse(404);
}
