#include <iostream>

#include "soundbag/StatefulServer.hpp"

using namespace soundbag;

StatefulServer::StatefulServer(const std::string& host, int port, const std::string& configFileName)
    : super(host, port, configFileName) {
  // nothing else to do
}

HttpResponse StatefulServer::handleApi(const HttpRequest& request) {
  std::string path = request.path().substr(5); // path always starts with "/api/"
  auto parsedPath = splitPath(path);
  if (parsedPath.size() == 2 && parsedPath[0] == "variable") {
    std::unique_lock lock(m_mutex);
    if (request.type() == HttpRequestType::POST) {
      m_variables[parsedPath[1]] = request.body();
    }
    if (m_variables.count(parsedPath[1])) {
      return HttpResponse(m_variables[parsedPath[1]]);
    }
  }

  return HttpResponse(404);
}
