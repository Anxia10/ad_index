#include "kernel/pool/variable_length_pool.h"
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
namespace kernel {
namespace pool {

LOG_SETUP("kernel", VariableLengthPool);

VariableLengthPool::VariableLengthPool() {
}

VariableLengthPool::~VariableLengthPool() {
}

bool VariableLengthPool::Write(const data::Addr& addr, const data::Data& data) {
    VariableDataHeader* header
        = reinterpret_cast<VariableDataHeader*>(addr.addr);
    if (unlikely(store_ == nullptr ||
                 header == nullptr)) {
        return false;
    }
    if (unlikely(static_cast<int32_t>(data.len)
                 > header->size)) {
        return false;
    }
    header->size = data.len;
    return !!(store_->Write(data::Addr(header + 1), data)).operate();
}

bool VariableLengthPool::Read(const data::Addr& addr, size_t len, data::Data* data) {
    VariableDataHeader* header =
        reinterpret_cast<VariableDataHeader*>(addr.addr);
    if (unlikely(store_ == nullptr ||
                 header == nullptr)) {
        LOG_ERROR("Input is nullptr.");
        return false;
    }
    char* c_addr = reinterpret_cast<char*>(addr.addr);
    return !!(store_->Read(data::Addr(c_addr + sizeof(VariableDataHeader)),header->size, data)).operate();
}

int64_t VariableLengthPool::NewData(const data::Data& data) {
    VariableDataHeader* header = reinterpret_cast<VariableDataHeader*>(
        Alloc(data.len));
    if (unlikely(header == nullptr || store_ == nullptr)) {
        LOG_ERROR("Input nullptr.");
        return -1L;
    }
    if (!(store_->Write(data::Addr(header + 1), data)).operate()) {
        LOG_ERROR("Write fail.");
        return -1L;
    }
    return GetDataOffset(header);
}

bool VariableLengthPool::DoFree(void* addr, size_t size) {
    VariableDataHeader* header =
        reinterpret_cast<VariableDataHeader*>(addr);
    if (unlikely(header == nullptr)) {
        LOG_ERROR("Input nullptr.");
        return false;
    }
    return MMapPool::DoFree(header, header->size + sizeof(VariableDataHeader));
}

void* VariableLengthPool::DoAlloc(size_t size) {
    VariableDataHeader* header = reinterpret_cast<VariableDataHeader*>(
        MMapPool::DoAlloc(size + sizeof(VariableDataHeader)));
    if (unlikely(header == nullptr)) {
        LOG_ERROR("Do alloc fail.");
        return nullptr;
    }
    header->size = size;
    return header;
}

} // namespace pool
} // namespace kernel