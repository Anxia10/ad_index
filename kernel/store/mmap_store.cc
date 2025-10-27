#include <sys/mman.h>
#include <string.h>
#include "kernel/store/mmap_store.h"
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

static const size_t kDefaultMMapSize = 1UL * 1024 * 1024 * 1024;

namespace kernel {
namespace store {

LOG_SETUP("kernel", MMapStore);

MMapStore::MMapStore() :
    base_(nullptr), mmap_size_(kDefaultMMapSize),
    memory_lock_(false), memory_preload_(false){
}

MMapStore::~MMapStore() {
    Close();
}

data::Status MMapStore::Open(const std::string& name, bool read_only) {
    data::Status status = FileStore::Open(name, read_only);
    if (!status.operate()) {
        LOG_ERROR("Open file %s fail.", name.c_str());
        return data::Status(data::StatusCode::Exception, "Open Fail");
    }
    int32_t mmap_port = read_only ? PROT_READ : PROT_READ | PROT_WRITE;
    int32_t flags = MAP_SHARED;
    if (memory_preload_) {
        flags |= MAP_POPULATE;
    }
    if (memory_lock_) {
        flags |= MAP_LOCKED;
    }
    char* base = reinterpret_cast<char*>(mmap(nullptr, mmap_size_, mmap_port, flags, fd_, 0));
    LOG_INFO("mmap start addr: %p", base);
    if (base == MAP_FAILED) {
        LOG_ERROR("File %s mmap fail. %s", name.c_str(), strerror(errno));
        return data::Status(data::StatusCode::Exception, "MMap Fail");
    }
    base_ = base;
    return data::Status(data::StatusCode::Success);
}

data::Status MMapStore::Close() {
    if (base_ != nullptr) {
        munmap(base_, mmap_size_);
        base_ = nullptr;
    }
    return FileStore::Close();
}

data::Status MMapStore::Read(const data::Addr& addr, size_t len, data::Data* data) {
    if (unlikely(base_ == nullptr)) {
        LOG_ERROR("MMap base is nullptr.");
        return data::Status(data::StatusCode::Exception, "Base nullptr");
    }
    char* read_begin = reinterpret_cast<char*>(addr.addr);
    char* mmap_end = base_ + mmap_size_;
    if (likely(!(read_begin >= base_ && read_begin + len <= mmap_end))) {
        LOG_ERROR("MMap [%s] read addr error. Base[%p] mmap size [%lu] addr [%p] len[%lu]", name_.c_str(), base_, mmap_size_, read_begin, len);
        return data::Status(data::StatusCode::Exception, "addr error");
    }
    data->data = addr.addr;
    data->len = len;
    return data::Status(data::StatusCode::Success);
}

data::Status MMapStore::Write(const data::Addr& addr, const data::Data& data) {
    if (unlikely(base_ == nullptr)) {
        LOG_ERROR("MMap base is nullptr.");
        return data::Status(data::StatusCode::Exception, "Base nullptr");
    }
    char* write_begin = reinterpret_cast<char*>(addr.addr);
    if (unlikely(write_begin < base_)) {
        LOG_ERROR("MMap write addr error. Base [%p] addr [%p]", base_, write_begin);
        return data::Status(data::StatusCode::Exception, "addr error");
    }
    size_t file_size = GetSize();
    char* mmap_end = base_ + (file_size < mmap_size_ ? file_size : mmap_size_); // file_size的大小没做变化

    if (unlikely(write_begin + data.len > mmap_end)) {
        LOG_DEBUG("Use FileStore Write. write_begin[%p], data len[%lu], mmap end[%p]", write_begin, data.len, mmap_end);
        size_t offset = write_begin - base_;
        return FileStore::Write(data::Addr(reinterpret_cast<void*>(offset)), data);
    }
    memcpy(write_begin, data.data, data.len);
    return data::Status(data::StatusCode::Success);
}

data::Status MMapStore::Append(const data::Data& data) {
    size_t begin = GetSize();
    data::Status status = Expand(data.len);
    if (!status.operate()) {
        return status;
    }
    return MMapStore::Write(data::Addr(base_ + begin), data);
}


}
}