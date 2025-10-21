#pragma once
#include "kernel/common/utils/utils.h"
namespace kernel {

void Utils::Split(const data::Data& data, const std::string& sep,
  std::vector<data::Data>* vec, bool ignore_empty) {
  vec->clear();
  size_t cmp_point = 0UL;
  const char* begin = reinterpret_cast<const char*>(data.data);
  const char* split_point = begin + data.len;
  const char* p = begin;
  for (size_t i = 0UL; i < data.len; i++, p++) {
    // match
    if (*p == sep[cmp_point]) {
      if (cmp_point == 0UL) {
        split_point = p;
      }
      // found
      if (cmp_point == sep.size() - 1) {
        int64_t len = split_point - begin;
        if (!ignore_empty || len > 0L) {
          vec->emplace_back(data::Data(begin, len));
        }
        cmp_point = 0UL;
        begin = p + 1;
      } else {
        cmp_point++;
      }
    // not match
    } else {
      cmp_point = 0UL;
    }
  }
  int64_t len = p - begin;
  if (!ignore_empty || len > 0L) {
    vec->emplace_back(data::Data(begin, len));
  }
}

void Utils::Split(const std::string& str,
  const std::string& sep,
  std::vector<data::Data>* vec, bool ignore_empty) {
  if (vec == nullptr) {
    return;
  }
  vec->clear();
  const char* begin = str.c_str();
  size_t size = str.size();
  size_t sep_size = sep.size();
  size_t start = 0UL;
  size_t len = 0UL;
  size_t found = str.find(sep, start);
  while (found != std::string::npos) {
    len = found - start;
    if (!ignore_empty || len > 0UL) {
      vec->emplace_back(data::Data(begin + start, len));
    }
    start = found + sep_size;
    found = str.find(sep, start);
  }
  len = size - start;
  if (!ignore_empty || len > 0UL) {
    vec->emplace_back(data::Data(begin + start, len));
  }
}

void Utils::Split(const std::string& str,
  const std::string& sep,
  std::vector<std::string>* vec, bool ignore_empty) {
  if (vec == nullptr) {
    return;
  }
  vec->clear();
  size_t size = str.size();
  size_t sep_size = sep.size();
  size_t start = 0;
  size_t found = str.find(sep, start);
  std::string splited;
  while (found != std::string::npos) {
    splited = str.substr(start, found - start);
    if (!ignore_empty || !splited.empty()) {
      vec->emplace_back(splited);
    }
    start = found + sep_size;
    found = str.find(sep, start);
  }
  splited = str.substr(start, size - start);
  if (!ignore_empty || !splited.empty()) {
    vec->emplace_back(splited);
  }
}

std::string Utils::Strip(const std::string& str) {
  const char* start = str.c_str();
  const char* end = start + str.size() - 1;
  while (start <= end &&
    ((*start) == ' ' || (*start) == '\n' ||
     (*start) == '\t' || (*start) == 0)) {
    start++;
  }
  while (start <= end &&
    ((*end) == ' ' || (*end) == '\n' ||
     (*end) == '\t' || (*end) == 0)) {
    end--;
  }
  if (start > end) return std::string();
  return str.substr(start - str.c_str(), end - start + 1);
}

std::string Utils::Join(const std::string& join_with,
  const std::string& section0,
  const std::string& section1,
  const std::string& section2,
  const std::string& section3,
  const std::string& section4,
  const std::string& section5) {
  std::string res;
  res.append(section0);
  if (!section1.empty()) {
    res.append(join_with);
    res.append(section1);
  }
  if (!section2.empty()) {
    res.append(join_with);
    res.append(section2);
  }
  if (!section3.empty()) {
    res.append(join_with);
    res.append(section3);
  }
  if (!section4.empty()) {
    res.append(join_with);
    res.append(section4);
  }
  if (!section5.empty()) {
    res.append(join_with);
    res.append(section5);
  }
  return res;
}

uint32_t Utils::MurmurHash2(const void* key, int32_t len, uint32_t seed) {
    // 'm' and 'r' are mixing constants generated offline.
    // They're not really 'magic', they just happen to work well.
    const uint32_t m = 0x5bd1e995;
    const int r = 24;
    // Initialize the hash to a 'random' value
    uint32_t h = seed ^ len;
    // Mix 4 bytes at a time into the hash
    const unsigned char * data = (const unsigned char *)key;
    while (len >= 4) {
        uint32_t k = *(reinterpret_cast<const uint32_t*>(data));
        k *= m;
        k ^= k >> r;
        k *= m;
        h *= m;
        h ^= k;
        data += 4;
        len -= 4;
    }
    // Handle the last few bytes of the input array
    switch (len) {
    case 3: h ^= data[2] << 16;
    case 2: h ^= data[1] << 8;
    case 1: h ^= data[0];
        h *= m;
    }
    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;
    return h;
}

} // namespace kernel
