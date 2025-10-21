#pragma once
#include <unordered_map>
#include <string>
#include "kernel/manager/table_info/field_info.h"

namespace kernel {
namespace manager {

struct IndexInfo {
  std::string name;
  std::string type;
  FieldInfo* field_info;
  std::string secondary_type;
  std::string metric;
  int32_t ncentroids;
  std::string bucket;
  int32_t dimension;
  int32_t threshold;
  int32_t pq_code_num;
  int32_t pq_nbits_per_code;
  int32_t segment_level;
  FieldDesc payload_info;
  int32_t ef_construction;
  int32_t ef_search;
  int32_t nlinks;

  static int32_t GetIndexSize(const IndexInfo& index_info);
  static int32_t GetPayloadSize(const IndexInfo& index_info);

  IndexInfo() : ncentroids(-1), dimension(-1), threshold(-1)
    , pq_code_num(-1), pq_nbits_per_code(-1), segment_level(-1)
    , ef_construction(200), ef_search(32), nlinks(32) {}

  explicit IndexInfo(const std::string& n,
    const std::string& t, FieldInfo* f = nullptr) : name(n),
    type(t), field_info(f) {}

  int32_t GetSize() const;
  int32_t GetPayloadSize() const;
  bool is_faiss;

  bool HasPayload() {
    return payload_info.empty();
  }

  std::string ToString() const {
    return "name=" + name + ",type=" + type + ",metric=" + metric
      + ",ncentroids=" + std::to_string(ncentroids)
      + ",bucket=" + bucket + ",dimension=" + std::to_string(dimension) + ",threshold=" + std::to_string(threshold)
      + ",pq_code_num=" + std::to_string(pq_code_num) + ",pq_nbits_per_code=" + std::to_string(pq_nbits_per_code)
      + ",is_faiss=" + (is_faiss ? "true" : "false");
  }
};

typedef std::unordered_map<std::string, IndexInfo> IndexDesc;

} // namespace manager
} // namespace kernel