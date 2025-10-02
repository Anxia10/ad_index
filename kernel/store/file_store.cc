#include "kernel/store/file_store.h"
#include <fcntl.h>
#include <unistd.h>
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

namespace kernel {
namespace store {

FileStore::FileStore() :
    fd_(-1), size_(0){
}

FileStore::~FileStore() {
    Close();
}

// 打开文件
data::Status FileStore::Open(const std::string& name, bool read_only) {
    if (fd_ >= 0) {
        // Log Error
        return data::Status(data::StatusCode::Exception, "already open");
    }

    if (read_only) {
        fd_ = open(name.c_str(), O_RDONLY); //只读
    } else {
        fd_ = open(name.c_str(), O_RDWR | O_CREAT | S_IRWXU | S_IRGRP | S_IROTH); 
    }

    if (fd_ < 0) {
        // Log Error
        return data::Status(data::StatusCode::Exception, "Open Fail");
    }

    size_ = GetFileSize();
    name_ = name;
    return data::Status(data::StatusCode::Success);
}

// 关闭文件
data::Status FileStore::Close() {
    if (fd_ < 0) {
        return data::Status(data::StatusCode::Success);
    }

    close(fd_);
    fd_ = -1;
    size_ = 0;
    name_.clear();

    return data::Status(data::StatusCode::Success);
}

// 不需要校验文件是否RDONLY？需要考虑指针和文件的内存偏移量转化是否有意义？
data::Status FileStore::Write(const data::Addr& addr, const data::Data& data) {
    if (unlikely(fd_ < 0)) {
        // Log Error
        return data::Status(data::StatusCode::Exception, "Not Open");
    }
    off_t pos = reinterpret_cast<off_t>(addr.addr); // 指针转为文件位置
    off_t pointer = lseek(fd_, pos, SEEK_SET); // 文件指针移动到addr的位置

    if(unlikely(pointer < 0 || pointer != pos)) {
        // Log Error
        return data::Status(data::StatusCode::Exception, "Lseek Fail");
    }

    off_t ret = write(fd_, reinterpret_cast<const char*>(data.data), data.len);
    if (unlikely(ret < 0)) {
        return data::Status(data::StatusCode::Exception, "Write Fail");
    }

    if (reinterpret_cast<size_t>(addr.addr) + data.len > size_) {
        size_ = reinterpret_cast<size_t>(addr.addr) + data.len;
    }
    return data::Status(data::StatusCode::Success);
}

data::Status FileStore::Read(const data::Addr& addr, size_t len, data::Data* data) {
    if (unlikely(fd_ < 0)) {
        // Log Error
        return data::Status(data::StatusCode::Exception, "Not Open");
    }

    off_t pos = reinterpret_cast<off_t>(addr.addr);
    off_t size = lseek(fd_, 0, SEEK_END);
    if (unlikely(pos > size)) {
        // Log Error
        return data::Status(data::StatusCode::Exception, "Pos Out Of Range");
    }

    off_t pointer = lseek(fd_, pos, SEEK_SET);
    size_t left_size = size - pointer;
    size_t read_len = left_size > len ? len : left_size;

    if(unlikely(pointer < 0 || pointer != pos)) {
        // Log Error
        return data::Status(data::StatusCode::Exception, "Lseek Fail");
    }

    off_t result_len = read(fd_, reinterpret_cast<char*>(const_cast<void *>(data->data)), read_len);
    
    if (read_len != result_len) {
        // Log Error
        return data::Status(data::StatusCode::Exception, "Length Exception");
    }
    data->len = read_len;

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
