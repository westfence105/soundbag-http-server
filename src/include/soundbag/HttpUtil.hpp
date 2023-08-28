#pragma once

#include <string>
#include <vector>

namespace soundbag {
  std::vector<std::string> splitPath(const std::string& path, char delim = '/');
  std::string joinPath(const std::vector<std::string>& path, char delim = '/');
}
