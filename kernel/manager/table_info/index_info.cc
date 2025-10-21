#include "kernel/manager/table_info/index_info.h"

namespace kernel {
namespace manager {

int32_t IndexInfo::GetIndexSize(const IndexInfo& index_info) {
  return GetPayloadSize(index_info) + sizeof(int64_t);
}

int32_t IndexInfo::GetPayloadSize(const IndexInfo& index_info) {
  int32_t size = 0;
  FieldDesc::const_iterator it
    = index_info.payload_info.begin();
  while (it != index_info.payload_info.end()) {
    int32_t local_size = it->second.GetSize();
    if (local_size <= 0) {
      return -1;
    }
    size += local_size;
    it++;
  }
  return size;
}

int32_t IndexInfo::GetPayloadSize() const {
  return GetPayloadSize(*this);
}

int32_t IndexInfo::GetSize() const {
  return GetIndexSize(*this);
}

} // namespace manager
} // namespace kernel