#pragma once
#include <stdio.h>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <iterator>

namespace kernel {
namespace util {
bool SplitStr(const std::string origin, const std::string symbol,
              std::vector<std::string> &container);
bool StrContain(const std::string& origin, const std::string& subStr);

template<typename Type>
std::string VectorJoin(std::vector<Type>& vec, const std::string& symbol) {
  std::ostringstream vts;
  if (!vec.empty()) {
    std::copy(vec.begin(), vec.end() - 1, std::ostream_iterator<Type>(vts, symbol.c_str()));
    vts << vec.back();
  }
  return vts.str();
}

template <typename Iterator>
std::string ArrayJoin(Iterator begin, Iterator end, char separator = ',') {
  std::ostringstream o;
  if(begin != end) {
    o << *begin++;
    for(;begin != end; ++begin)
      o << separator << *begin;
  }
  return o.str();
}

template <typename Container>
std::string ArrayJoin(Container const& c, char separator = ',') { 
  return ArrayJoin(std::begin(c), std::end(c), separator);
}

template <typename Type>
void TypeToString(Type typeTmp, std::string& strTmp) {
  std::stringstream stream;
  stream << typeTmp;
  stream >> strTmp;
  stream.clear();
  stream.str("");
}

template <typename Type>
void StringToType(std::string strTmp, Type& typeTmp) {
  std::stringstream stream;
  stream << strTmp;
  stream >> typeTmp;
  stream.clear();
  stream.str("");
}

} // namespace util
} // namespace kernel