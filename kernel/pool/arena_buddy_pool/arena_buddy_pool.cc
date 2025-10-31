#include <unistd.h>
#include "kernel/pool/arena_buddy_pool/arena_buddy_pool.h"
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

namespace kernel {
namespace pool {

LOG_SETUP("kernel", AreanaBuddyPool);

AreanaBuddyPool::AreanaBuddyPool() {
}

AreanaBuddyPool::~AreanaBuddyPool() {
}

bool AreanaBuddyPool::Init(const std::string& file_name,
                          bool read_only) {
    MMapPool::Init(file_name, read_only);
    return InitPool();
}

bool AreanaBuddyPool::InitPool() {
    return heap_mem_manager_.Init(this, GetMMapDataBegin());
}

void* AreanaBuddyPool::DoAlloc(size_t size) {
    int64_t ret = heap_mem_manager_.AllocData(size
        + sizeof(AreanaBuddyDataHeader));
    if (unlikely(ret == ERROR_ALLOC_ADDR)) {
        LOG_ERROR("Alloc Error.");
        return nullptr;
    }
    AreanaBuddyDataHeader* header = reinterpret_cast<AreanaBuddyDataHeader*>(ret);
    if (unlikely(header == nullptr || store_ == nullptr)) {
        LOG_ERROR("Input nullptr.");
        return nullptr;
    }
    header->size = size;
    return reinterpret_cast<void*>(ret + sizeof(AreanaBuddyDataHeader));
}

int64_t AreanaBuddyPool::NewDataLen(const Data& data) {
    int64_t ret = heap_mem_manager_.AllocData(data.len
        + sizeof(AreanaBuddyDataHeader));
    AreanaBuddyDataHeader* header = reinterpret_cast<AreanaBuddyDataHeader*>(ret);
    if (unlikely(header == nullptr || store_ == nullptr)) {
        LOG_ERROR("Input nullptr.");
        return -1L;
    }
    header->size = data.len;
    if (!store_->Write(Addr(header + 1), data)) {
        LOG_ERROR("Write fail.");
        return -1L;
    }
    return ret + sizeof(AreanaBuddyDataHeader);
}

int64_t AreanaBuddyPool::NewData(const Data& data) {
    void* addr = Alloc(data.len);
    if (addr == nullptr) {
        LOG_ERROR("NewData Error.");
        return -1L;
    }
    AreanaBuddyDataHeader* header =
        reinterpret_cast<AreanaBuddyDataHeader*>(addr) - 1;
    if (unlikely(header == nullptr || store_ == nullptr)) {
        LOG_ERROR("Input nullptr.");
        return -1L;
    }
    header->size = data.len;
    if (!store_->Write(Addr(header + 1), data)) {
        LOG_ERROR("Write fail.");
        return -1L;
    }
    return GetDataOffset(header + 1);
}

bool AreanaBuddyPool::DoFree(void* addr, size_t size) {
    void* addr_t = reinterpret_cast<char*>(addr) - sizeof(AreanaBuddyDataHeader);
    AreanaBuddyDataHeader* header =
        reinterpret_cast<AreanaBuddyDataHeader*>(addr_t);
    if (unlikely(header == nullptr || store_ == nullptr)) {
        LOG_ERROR("Input nullptr.");
        return false;
    }
    return heap_mem_manager_.Free(GetDataOffset(addr_t),
        header->size + sizeof(AreanaBuddyDataHeader));
}

bool AreanaBuddyPool::Write(const Addr& addr, const Data& data) {
    AreanaBuddyDataHeader* header
        = reinterpret_cast<AreanaBuddyDataHeader*>(
            reinterpret_cast<char*>(addr.addr) - sizeof(AreanaBuddyDataHeader));
    if (unlikely(store_ == nullptr ||
        header == nullptr)) {
        return false;
    }
    if (unlikely(static_cast<int32_t>(data.len)
        > header->size)) {
        return false;
    }
    header->size = data.len;
    return !!store_->Write(Addr(header + 1), data);
}

bool AreanaBuddyPool::Read(const Addr& addr, size_t len, Data* data) {
    AreanaBuddyDataHeader* header =
        reinterpret_cast<AreanaBuddyDataHeader*>(
            reinterpret_cast<char*>(addr.addr) - sizeof(AreanaBuddyDataHeader));
    if (unlikely(store_ == nullptr ||
        header == nullptr)) {
        LOG_ERROR("Input is nullptr.");
        return false;
    }
    char* c_addr = reinterpret_cast<char*>(addr.addr);
    return !!store_->Read(
        Addr(c_addr),
        header->size, data);
}

void* AreanaBuddyPool::AllocNewStore(size_t size) {
    return MMapPool::AllocFromStore(size);
}

} // namespace pool
} // namespace kernel