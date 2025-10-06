#pragma once
#include "kernel/pool/mmap_pool.h"
namespace kernel {
namespace pool {

struct VariableDataHeader {
    int32_t size;
};

class VariableLengthPool : public MMapPool {
    public:
        VariableLengthPool();
        virtual ~VariableLengthPool();
        bool Write(const data::Addr& addr, const data::Data& data) override;
        void* DoAlloc(size_t size) override;
        int64_t NewData(const data::Data& data) override;
        bool Read(const data::Addr& addr, size_t len, data::Data* data) override;
        bool DoFree(void* addr, size_t size) override;
    private:
        LOG_DECLARE;
};

} 
} 