#pragma once
#include <sys/time.h>
#include <queue>
#include <map>
#include <mutex>
#include "kernel/pool/pool.h"
#include "kernel/store/file_store.h"
#include "kernel/common/concurrency/lock_free_queue.hpp"
#include "kernel/common/concurrency/thread_pool.h"

namespace kernel {
namespace pool {

struct Block {
    void* addr;
    uint32_t timestamp;
    uint32_t size;
    bool operator<(const Block& b) const {
        return reinterpret_cast<size_t>(addr) < reinterpret_cast<size_t>(b.addr);
    }
};

class FilePool : public Pool {
    public:
        struct FreeListMeta {
            FreeListMeta() :
                user_space_size(0UL), block_size(0UL) {}
            size_t user_space_size;
            size_t block_size;
        };

        FilePool();
        virtual ~FilePool();

        bool Init(const std::string& file_name,
                bool read_only = false) override;
        void* Alloc(size_t size) final;
        virtual void* DoAlloc(size_t size);
        bool Free(void* addr, size_t size) final;
        virtual bool DoFree(void* addr, size_t size);
        bool Write(const data::Addr& addr, const data::Data& data) override;
        bool Read(const data::Addr& addr, size_t len, data::Data* data) override;
        int64_t NewData(const data::Data& data) override;
        void Release() override;
        size_t GetPoolSize() override {
            return store_->GetSize();
        }

        size_t GetUsedSize() override {
            return store_->GetSize();
        }

        size_t GetFreeListSize() {
            return free_list_meta_.user_space_size;
        }
        void EnableFreeList() {
            use_free_list_ = true;
        }
        void DisableFreeList() {
            use_free_list_ = false;
        }
        void SetReuseTimegap(int32_t timegap) {
            reuse_timegap_ = timegap;
        }
        int32_t GetReuseTimegap() {
            return reuse_timegap_;
        }

    protected:
        virtual size_t GetAlignedSize(size_t size) {
            return size;
        }
        virtual void* AllocFromFreeList(size_t size);
        virtual void* AllocFromStore(size_t size);
        virtual bool LoadFreeList(const std::string& free_list_file);
        virtual bool DumpFreeList(const std::string& free_list_file);

    private:
        FilePool(const FilePool& pool) = default;
        void operator=(const FilePool& pool) {}
        void operator=(const FilePool&& pool) {}

    protected:
        bool use_free_list_;
        int32_t reuse_timegap_;
        FreeListMeta free_list_meta_;
        typedef std::map<size_t, std::priority_queue<Block>> FreeList;
        FreeList free_list_;
        std::mutex alloc_free_mutex_;
        concurrency::ThreadPool delay_free_thread_pool_;

    private:
        LOG_DECLARE;
};

}
}