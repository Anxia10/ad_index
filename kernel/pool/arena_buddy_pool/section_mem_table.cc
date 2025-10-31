#include "kernel/pool/arena_buddy_pool/section_mem_table.h"

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

namespace kernel {
namespace pool {

SectionMemTable::SectionMemTable() :
    head_(nullptr), begin_(nullptr) {
}

SectionMemTable::~SectionMemTable() {
}

bool SectionMemTable::Init(uint64_t addr, char* begin, size_t size) {
    uint64_t start = addr;
    head_ = reinterpret_cast<SectionMemTableHead*>(begin + addr);
    head_->Init(start, size);
    begin_ = begin;
    return true;
}

bool SectionMemTable::InitTable(uint64_t addr, char* begin, size_t size) {
    head_ = reinterpret_cast<SectionMemTableHead*>(begin + addr);
    begin_ = begin;
    return true;
}

int64_t SectionMemTable::AllocData(const size_t& len) {
    if (head_->valid_flag) {
        if (head_->current + len > head_->end) {
            return ERROR_ALLOC_ADDR;
        }
        uint64_t ret = head_->current;
        head_->current += len;
        return reinterpret_cast<int64_t>(GetAddr(ret));
    }
    return ERROR_ALLOC_ADDR;
}

} // namespace pool
} // namespace kernel