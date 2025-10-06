#pragma once
#include "kernel/pool/mmap_pool.h"

namespace kernel {
namespace pool {

class FixLengthPool : public MMapPool {
public:
    explicit FixLengthPool(int32_t value_size = 0);
    virtual ~FixLengthPool();
    bool Write(const data::Addr& addr, const data::Data& data) override;
    int64_t NewData(const data::Data& data) override;
    bool Read(const data::Addr& addr, size_t len, data::Data* data) override;
    bool DoFree(void* addr, size_t size) override;
    void* DoAlloc(size_t size) override;

    void SetValueSize(int32_t value_size) {
        value_size_ = value_size;
    }
    int32_t GetValueSize() {
        return value_size_;
    }
private:
    int32_t value_size_;
    LOG_DECLARE;
};

}
}