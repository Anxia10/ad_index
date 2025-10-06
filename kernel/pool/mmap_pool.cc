#include <unistd.h>
#include <string>
#include <queue>
#include "kernel/pool/mmap_pool.h"
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

namespace kernel {
namespace pool {

static const size_t kDefaultMinExpandSize = 1UL * 1024 * 1024;

LOG_SETUP("kernel", MMapPool);

MMapPool::MMapPool() :
    begin_(nullptr), min_expand_size_(kDefaultMinExpandSize) {
}

MMapPool::~MMapPool() {
    if (store_ != nullptr) {
        delete store_;
        store_ = nullptr;
    }
}

bool MMapPool::Init(const std::string& file_name,
                    bool read_only) {
    read_only_ = read_only;
    if (store_ == nullptr) {
        store_ = new store::MMapStore();
    }
    if (store_ == nullptr) {
        LOG_ERROR("Store alloc exception.");
        return false;
    }
    data::Status status = store_->Open(file_name, read_only);
    if (!status.operate()) {
        LOG_ERROR("Store open exception. Reason [%s].",
                  status.GetReason().c_str());
        return false;
    }
    begin_ = reinterpret_cast<store::MMapStore*>(store_)->GetBase();
    if (store_->GetSize() <= 0) {
        store_->Expand(min_expand_size_);
        *(reinterpret_cast<size_t*>(begin_)) = 64UL;
    }
    std::string free_list_file = store_->GetName() + ".fl";
    if (0 == access(free_list_file.c_str(), R_OK)) {
        LOG_INFO("Found free list file. Load free list now.");
        if (!LoadFreeList(free_list_file)) {
            LOG_WARN("Load free list fail.");
        }
    }
    return true;
}

void* MMapPool::AllocFromStore(size_t size) {
    void* ret = nullptr;
    if (unlikely(store_ == nullptr)) {
        LOG_ERROR("Store alloc exception.");
        return ret;
    }
    while (GetUsedSize() + size >= store_->GetSize()) {
        data::Status status = store_->Expand(min_expand_size_);
        if (unlikely(!status.operate())) {
            LOG_ERROR("Expand file fail.");
            return nullptr;
        }
    }
    ret = begin_ + GetUsedSize();
    *(reinterpret_cast<size_t*>(begin_)) += size;
    return ret;
}

bool MMapPool::LoadFreeList(const std::string& free_list_file) {
    store::FileStore* store = new store::FileStore();
    if (store == nullptr) {
        LOG_WARN("Create file store fail.");
        return false;
    }
    if (!(store->Open(free_list_file)).operate()) {
        LOG_WARN("Open file store fail.");
        delete store;
        return false;
    }
    data::Addr addr;
    data::Data data;
    char* pos = 0;
    free_list_.clear();
    size_t size = 0;
    size_t len = 0;
    addr.addr = pos;
    data.data = &size;
    len = sizeof(size);
    if (!(store->Read(addr, len, &data)).operate()) {
        LOG_WARN("Read size fail.");
        store->Close();
        delete store;
        return false;
    }
    pos += data.len;
    Block block;
    for (size_t i = 0UL; i < size; i++) {
        addr.addr = pos;
        data.data = &block;
        len = sizeof(block);
        if (!(store->Read(addr, len, &data)).operate()) {
            LOG_WARN("Read block fail.");
            continue;
        }
        block.addr = GetMMapDataBegin()
                     + reinterpret_cast<size_t>(block.addr);
        free_list_[block.size].push(block);
        free_list_meta_.user_space_size += block.size;
        pos += data.len;
    }
    store->Close();
    delete store;
    return true;
}

bool MMapPool::DumpFreeList(const std::string& free_list_file) {
    if (read_only_ || free_list_meta_.user_space_size == 0UL) return true;
    store::FileStore* store = new store::FileStore();
    if (store == nullptr) {
        LOG_WARN("Create file store fail.");
        return false;
    }
    if (!(store->Open(free_list_file, false)).operate()) {
        LOG_WARN("Open file store fail.");
        delete store;
        return false;
    }
    data::Addr addr;
    data::Data data;
    size_t size = 0UL;
    char* pos = reinterpret_cast<char*>(sizeof(size));
    for (FreeList::iterator it = free_list_.begin(); it != free_list_.end();
         it++) {
        std::priority_queue<Block>& list = it->second;
        while (!list.empty()) {
            Block block = list.top();
            size++;
            size_t offset = reinterpret_cast<char*>(block.addr)
                           - GetMMapDataBegin();
            block.addr = reinterpret_cast<void*>(offset);
            addr.addr = pos;
            data.data = &block;
            data.len = sizeof(block);
            if (!(store->Write(addr, data)).operate()) {
                LOG_WARN("Write block fail.");
                continue;
            }
            pos += data.len;
            list.pop();
        }
    }
    addr.addr = 0;
    data.data = &size;
    data.len = sizeof(size);
    if (!(store->Write(addr, data)).operate()) {
        LOG_WARN("Write size fail.");
        store->Close();
        delete store;
        return false;
    }
    free_list_.clear();
    pos = reinterpret_cast<char*>(sizeof(size) + size * sizeof(Block));
    free_list_meta_.user_space_size = 0UL;
    if (!(store->Truncate(reinterpret_cast<size_t>(pos))).operate()) {
        LOG_WARN("Truncate fail.");
    }
    store->Close();
    delete store;
    return true;
}

int64_t MMapPool::NewData(const data::Data& data) {
    if (data.len <= 0) {
        return -1L;
    }
    void* addr = Alloc(data.len);
    if (addr == nullptr) {
        return -1L;
    }
    if (!Write(data::Addr(addr), data)) {
        return -1L;
    }
    return GetDataOffset(addr);
}


}
}