#pragma once
#include <string>
#include "kernel/pool/map_pool.h"
#include "kernel/pool/arena_buddy_pool/ab_pool_define.h"
#include "kernel/pool/arena_buddy_pool/heap_mem_manager.h"

namespace kernel {
namespace pool {

struct AreanaBuddyDataHeader {
    int32_t size;
};

class AreanaBuddyPool : public MMapPool {
public:
    AreanaBuddyPool();
    virtual ~AreanaBuddyPool();

    bool Init(const std::string& file_name,
              bool read_only = false) override;
    bool Write(const Addr& addr, const Data& data) override;
    int64_t NewData(const Data& data) override;
    bool Read(const Addr& addr, size_t len, Data* data) override;
    bool DoFree(void* addr, size_t size) override;
    void* DoAlloc(size_t size) override;

    bool InitPool();
    int64_t NewDataLen(const Data& data);
    void* AllocNewStore(size_t size);

private:
    HeapMemManager heap_mem_manager_;
    LOG_DECLARE;
};

} // namespace pool
} // namespace kernel