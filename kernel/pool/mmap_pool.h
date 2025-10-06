#pragma once
#include <string>
#include "kernel/pool/file_pool.h"
#include "kernel/store/mmap_store.h"

namespace kernel {
namespace pool {

class MMapPool : public FilePool {
    public:
        MMapPool();
        virtual ~MMapPool();
        bool Init(const std::string& file_name,
                bool read_only = false) override;
        int64_t NewData(const data::Data& data) override;
        void SetMinExpandSize(size_t size) {
            min_expand_size_ = size;
        }
        size_t GetUsedSize() override {
            return begin_ == nullptr ?
                0UL : *(reinterpret_cast<size_t*>(begin_));
        }
        size_t GetDataSize() const {
            return begin_ == nullptr ?
                0UL : *(reinterpret_cast<size_t*>(begin_)) - 64UL;
        }
        size_t GetDataOffset(const void* addr) {
            return reinterpret_cast<const char*>(addr) - begin_ - 64UL;
        }
        char* GetDataAddr(size_t offset) {
            return begin_ + 64UL + offset;
        }
        void Resize(size_t size) {
            *(reinterpret_cast<size_t*>(begin_)) = 64UL + size;
        }
        bool Empty() {
            return GetDataSize() == 0UL;
        }
        char* GetMMapBegin() {
            return begin_;
        }
        char* GetMMapDataBegin() {
            return begin_ + 64UL;
        }

    protected:
        void* AllocFromStore(size_t size) override;
        bool LoadFreeList(const std::string& free_list_file) override;
        bool DumpFreeList(const std::string& free_list_file) override;

    private:
        MMapPool(const MMapPool& pool) = default;
        MMapPool(const MMapPool&& pool) {}
        void operator=(const MMapPool& pool) {}
        void operator=(const MMapPool&& pool) {}

    private:
        char* begin_;
        size_t min_expand_size_;
        LOG_DECLARE;
};

}
}