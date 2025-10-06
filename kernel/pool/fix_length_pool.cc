#include <unistd.h>
#include <string>
#include <queue>
#include "kernel/pool/fix_length_pool.h"
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

namespace kernel {
namespace pool {

LOG_SETUP("kernel", FixLengthPool);

FixLengthPool::FixLengthPool(int32_t value_size) :
    value_size_(value_size) {
}

FixLengthPool::~FixLengthPool() {
}

bool FixLengthPool::Write(const data::Addr& addr, const data::Data& data) {
    if (unlikely(store_ == nullptr || value_size_ <= 0 ||
                 data.len != static_cast<size_t>(value_size_))) {
        return false;
    }
    return !!(store_->Write(addr, data)).operate();
}

bool FixLengthPool::Read(const data::Addr& addr, size_t len, data::Data* data) {
    if (unlikely(store_ == nullptr || value_size_ <= 0)) {
        return false;
    }
    return !!(store_->Read(addr, value_size_, data)).operate();
}

void* FixLengthPool::DoAlloc(size_t size) {
    if (unlikely(size != static_cast<size_t>(value_size_))) {
        return nullptr;
    }
    return MMapPool::DoAlloc(size);
}

int64_t FixLengthPool::NewData(const data::Data& data) {
    if (unlikely(store_ == nullptr || value_size_ <= 0)) {
        return -1L;
    }
    void* addr = Alloc(data.len);
    if (unlikely(addr == nullptr)) {
        return -1L;
    }
    if (!(store_->Write(data::Addr(addr), data)).operate()) {
        LOG_ERROR("Write fail.");
        return -1L;
    }
    return GetDataOffset(addr);
}

bool FixLengthPool::DoFree(void* addr, size_t size) {
    if (unlikely(value_size_ <= 0)) {
        return false;
    }
    return MMapPool::DoFree(addr, value_size_);
}

}
}