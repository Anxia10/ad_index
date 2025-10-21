#include "kernel/common/utils/string_util.h"

namespace kernel {
namespace util {
bool SplitStr(const std::string origin, const std::string symbol,
              std::vector<std::string> &container) {
  if (origin.empty() || symbol.empty()) {
    return false;
  }
  container.clear();
  char *buffer = new char[origin.size() + 1];
  strcpy(buffer, origin.c_str());
  char *tmp;
  char *p = strtok_r(buffer, symbol.c_str(), &tmp);  // 第一次分割
  do {
    container.push_back(p);  // 如果 p 为 nullptr，则将整个字符串作为结果
  } while ((p = strtok_r(nullptr, symbol.c_str(), &tmp)) != nullptr);
  // strtok_r 为 strtok 的线程安全版本。
  delete[] buffer;
  return true;
}

bool StrContain(const std::string& origin, const std::string& subStr) {
  std::size_t found = origin.find(subStr);
  if (found != std::string::npos) {
    return true;
  }
  return false;
}

} // namespace util
} // namespace kernel