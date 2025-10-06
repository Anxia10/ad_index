#include <unistd.h>
#include <string>
#include "kernel/pool/file_pool.h"
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

namespace kernel {
namespace pool {

static const int32_t kDefaultTimegap = 3;
static const size_t kInitReserveSpace = 64;

LOG_SETUP("kernel", FilePool);

FilePool::FilePool() :
    use_free_list_(true),
    reuse_timegap_(kDefaultTimegap),
    delay_free_thread_pool_(1UL, -1UL) {
    delay_free_thread_pool_.Start();
}

FilePool::~FilePool() {
    Release();
}

void FilePool::Release() {
    delay_free_thread_pool_.Stop();
    if (store_ != nullptr) {
        std::string free_list_file = store_->GetName() + ".fl";
        if (!DumpFreeList(free_list_file)) {
            LOG_WARN("Dump free list fail.");
        }
        store_->Close();
        delete store_;
        store_ = nullptr;
    }
}

bool FilePool::Init(const std::string& file_name, bool read_only) {
    read_only_ = read_only;
    if (store_ == nullptr) {
        store_ = new store::FileStore();
    }
    if (store_ == nullptr) {
        LOG_ERROR("Store alloc exception.");
        return false;
    }
    data::Status status = store_->Open(file_name, read_only);
    if (!status.operate()) {
        LOG_ERROR("Store open exception. Reason [%s].", status.GetReason().c_str());
        return false;
    }
    if (store_->GetSize() <= 0) {
        status = store_->Expand(kInitReserveSpace);
        if (!status.operate()) {
            LOG_ERROR("Store expand fail.");
            return false;
        }
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

void* FilePool::Alloc(size_t size) {
    std::lock_guard<std::mutex> lock(alloc_free_mutex_);
    return DoAlloc(size);
}

void* FilePool::DoAlloc(size_t size) {
    size = GetAlignedSize(size);
    void* ret = nullptr;
    if (use_free_list_) {
        ret = AllocFromFreeList(size);
    }
    if (ret != nullptr) {
        return ret;
    }
    return AllocFromStore(size);
}

int64_t FilePool::NewData(const data::Data& data) {
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
    return reinterpret_cast<int64_t>(addr);
}

bool FilePool::Free(void* addr, size_t size) {
    if (reuse_timegap_ <= 0) return DoFree(addr, size);
    struct timeval call_tv;
    gettimeofday(&call_tv, nullptr);
    int64_t call_time = call_tv.tv_sec * 1000000 + call_tv.tv_usec;
    concurrency::ThreadTask task = [this, call_time, addr, size]() {
        struct timeval exe_tv;
        gettimeofday(&exe_tv, nullptr);
        int64_t exe_time = exe_tv.tv_sec * 1000000 + exe_tv.tv_usec;
        int64_t delay_time = 1000000L * reuse_timegap_ - (exe_time - call_time);
        if (delay_time > 0L) {
            usleep(delay_time);
        }
        std::lock_guard<std::mutex> lock(alloc_free_mutex_);
        DoFree(addr, size);
    };
    delay_free_thread_pool_.AddTask(std::move(task));
    return true;
}

bool FilePool::DoFree(void* addr, size_t size) {
    size = GetAlignedSize(size);
    if (!use_free_list_) {
        return true;
    }
    Block block;
    block.size = size;
    block.addr = addr;
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    free_list_[size].push(block);
    free_list_meta_.user_space_size += block.size;
    free_list_meta_.block_size++;
    return true;
}

bool FilePool::Write(const data::Addr& addr, const data::Data& data) {
    if (unlikely(store_ == nullptr)) {
        LOG_ERROR("Store alloc exception.");
        return false;
    }
    data::Status status = store_->Write(addr, data);
    if (unlikely(!status.operate())) {
        LOG_ERROR("Store write exception.");
        return false;
    }
    return true;
}

bool FilePool::Read(const data::Addr& addr, size_t len, data::Data* data) {
    if (unlikely(store_ == nullptr)) {
        LOG_ERROR("Store alloc exception.");
        return false;
    }
    data::Status status = store_->Read(addr, len, data);
    if (unlikely(!status.operate())) {
        LOG_ERROR("Store read exception.");
        return false;
    }
    return true;
}

void* FilePool::AllocFromFreeList(size_t size) {
    FreeList::iterator list_it = free_list_.find(size);
    if (list_it == free_list_.end() || list_it->second.empty()) {
        return nullptr;
    }
    std::priority_queue<Block>& list = list_it->second;
    Block block = list.top();
    void* ret = block.addr;
    list.pop();
    if (list.empty()) {
        free_list_.erase(size);
    }
    free_list_meta_.block_size--;
    free_list_meta_.user_space_size -= size;
    return ret;
}

void* FilePool::AllocFromStore(size_t size) {
    void* ret = nullptr;
    if (unlikely(store_ == nullptr)) {
        LOG_ERROR("Store alloc exception.");
        return ret;
    }
    size_t pos = store_->GetSize();
    data::Status status = store_->Expand(size);
    if (unlikely(!status.operate())) {
        LOG_ERROR("Store expand fail.");
        return ret;
    }
    ret = reinterpret_cast<void*>(pos);
    return ret;
}

bool FilePool::LoadFreeList(const std::string& free_list_file) {
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
        free_list_[block.size].push(block);
        free_list_meta_.user_space_size += block.size;
        pos += data.len;
    }
    store->Close();
    delete store;
    return true;
}

bool FilePool::DumpFreeList(const std::string& free_list_file) {
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
    store->Close();
    delete store;
    return true;
}

}
}