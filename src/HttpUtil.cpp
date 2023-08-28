#include <sstream>

#include "soundbag/HttpUtil.hpp"

std::vector<std::string> soundbag::splitPath(const std::string& path, char delim) {
  std::vector<std::string> parsedPath;
  size_t offset = 0;
  while (offset < path.length()) {
    size_t pos = path.find(delim, offset);
    if (pos == std::string::npos) {
      parsedPath.push_back(path.substr(offset));
      break;
    }
    else {
      parsedPath.push_back(path.substr(offset, pos - offset));
      offset = pos + 1;
    }
  }

  return parsedPath;
}

std::string soundbag::joinPath(const std::vector<std::string>& path, char delim) {
  if (path.size() == 0) {
    return "";
  }
  else {
    std::ostringstream oss;
    for (size_t i = 0; i < path.size() - 1; ++i) {
      oss << path[i] << delim;
    }
    oss << path[path.size()-1];
    return oss.str();
  }
}
