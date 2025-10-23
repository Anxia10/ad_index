#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "kernel/store/file_store.h"
#include "kernel/common/data/data.h"
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

namespace kernel {
namespace store {

LOG_SETUP("kernel", FileStore);

FileStore::FileStore() :
    fd_(-1), size_(0){
}

FileStore::~FileStore() {
    Close();
}

// 打开文件
data::Status FileStore::Open(const std::string& name, bool read_only) {
    if (fd_ >= 0) {
        LOG_ERROR("File %s already open.", name.c_str());
        return data::Status(data::StatusCode::Exception, "already open");
    }
    if (read_only) {
        fd_ = open(name.c_str(), O_RDONLY); //只读
    } else {
        fd_ = open(name.c_str(), O_RDWR | O_CREAT , S_IRWXU | S_IRGRP | S_IROTH); // 可读、可写、可执行
    }

    if (fd_ < 0) {
        LOG_ERROR("File %s open fail.[%s]", name.c_str(), strerror(errno));
        return data::Status(data::StatusCode::Exception, "Open Fail");
    }

    size_ = GetFileSize();
    name_ = name;
    return data::Status(data::StatusCode::Success);
}

// 关闭文件
data::Status FileStore::Close() {
    if (fd_ < 0) {
        LOG_INFO("File %s aleady Close.", name_.c_str());
        return data::Status(data::StatusCode::Success);
    }

    close(fd_);
    fd_ = -1;
    size_ = 0;
    name_.clear();

    return data::Status(data::StatusCode::Success);
}

data::Status FileStore::Write(const data::Addr& addr, const data::Data& data) {
    if (unlikely(fd_ < 0)) {
        LOG_ERROR("File not open.");
        return data::Status(data::StatusCode::Exception, "Not Open");
    }
    off_t pos = reinterpret_cast<off_t>(addr.addr); // 指针转为文件位置
    // LOG_INFO("Addr %ld. Get pos %ld", addr.addr, pos); // Write Addr 6. Get pos 6
    off_t pointer = lseek(fd_, pos, SEEK_SET); // 文件指针移动到addr的位置

    if(unlikely(pointer < 0 || pointer != pos)) {
        LOG_ERROR("File %s lseek pos %ld exception. Get pos %ld", GetName().c_str(), pos, pointer);
        return data::Status(data::StatusCode::Exception, "Lseek Fail");
    }

    off_t ret = write(fd_, reinterpret_cast<const char*>(data.data), data.len);
    if (unlikely(ret < 0)) {
        LOG_ERROR("File %s write exception. Data addr %p len %ld", GetName().c_str(), data.data, data.len);
        return data::Status(data::StatusCode::Exception, "Write Fail");
    }

    if (reinterpret_cast<size_t>(addr.addr) + data.len > size_) {
        size_ = reinterpret_cast<size_t>(addr.addr) + data.len;
    }
    return data::Status(data::StatusCode::Success);
}

data::Status FileStore::Append(const data::Data& data) {
    data::Addr addr;
    addr.addr = reinterpret_cast<void*>(GetSize()); 
    if (unlikely(reinterpret_cast<off_t>(addr.addr) < 0)) {
        LOG_ERROR("File get size exception.");
        return data::Status(data::StatusCode::Exception, "get size exception");
    }
    return FileStore::Write(addr, data);
}

data::Status FileStore::Read(const data::Addr& addr, size_t len, data::Data* data) {
    if (unlikely(fd_ < 0)) {
        LOG_ERROR("File not open.");
        return data::Status(data::StatusCode::Exception, "Not Open");
    }

    off_t pos = reinterpret_cast<off_t>(addr.addr);
    off_t size = lseek(fd_, 0, SEEK_END);
    if (unlikely(pos > size)) {
        LOG_ERROR("File Read Pos Out Of Range");
        return data::Status(data::StatusCode::Exception, "Pos Out Of Range");
    }

    off_t pointer = lseek(fd_, pos, SEEK_SET);
    size_t left_size = size - pointer;
    size_t read_len = left_size > len ? len : left_size;

    if(unlikely(pointer < 0 || pointer != pos)) {
        LOG_ERROR("File Read Lseek Exception");
        return data::Status(data::StatusCode::Exception, "Lseek Fail");
    }

    off_t result_len = read(fd_, reinterpret_cast<char*>(const_cast<void *>(data->data)), read_len);
    
    if (read_len != result_len) {
        LOG_ERROR("File Read Len Exception");
        return data::Status(data::StatusCode::Exception, "Length Exception");
    }
    data->len = read_len;

    return data::Status(data::StatusCode::Success);
}

data::Status FileStore::Expand(size_t size) {
    if (unlikely(size == 0)) {
        LOG_DEBUG("File expand size 0.");
        return data::Status(data::StatusCode::Success);
    }
    if (unlikely(fd_ < 0)) {
        LOG_ERROR("File not open.");
        return data::Status(data::StatusCode::Exception, "not open");
    }
    off_t pos = lseek(fd_, size - 1, SEEK_END); // 将文件指针移动到 “当前文件末尾 + (size - 1) 字节” 的位置
    if (unlikely(pos < 0)) {
        LOG_ERROR("File expand fail.");
        return data::Status(data::StatusCode::Exception, "expand size exception");
    }
    pos = write(fd_, &size, 1); // 尝试写一个文字？
    if (unlikely(pos < 0)) {
        LOG_ERROR("File write with expand fail.");
        return data::Status(data::StatusCode::Exception, "expand size exception");
    }
    size_ += size;
     return data::Status(data::StatusCode::Success);
 }

 data::Status FileStore::Truncate(size_t size) {
    if (0 != ftruncate(fd_, size)) {
        return data::Status(data::StatusCode::Exception, "ftruncate exception");
    }
    return data::Status(data::StatusCode::Success);
 }

size_t FileStore::GetFileSize() {
    if (unlikely(fd_ < 0)) {
        // Log Error
        return -1L;
    }
    return lseek(fd_, 0, SEEK_END);
}

}
}
