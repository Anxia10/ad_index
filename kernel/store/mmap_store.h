#pragma once
#include "kernel/store/file_store.h"

namespace kernel {
namespace store {

class MMapStore : public FileStore {
    public:
        MMapStore();
        virtual ~MMapStore();

        data::Status Open(const std::string& name, bool read_only = false) override;
        data::Status Close() override;
        data::Status Read(const data::Addr& addr, size_t len, data::Data* data) override;
        data::Status Write(const data::Addr& addr, const data::Data& data) override;
        data::Status Append(const data::Data& data) override;

        char * GetBase() {
            return base_;
        }
        void SetMMapSize(size_t mmap_size) {
            mmap_size_ = mmap_size;
        }
        size_t GetMMapSize() {
            return mmap_size_;
        }
        void SetMemoryLock(bool memory_force) {
            memory_lock_ = memory_force;
        }
        void SetMemoryPreload(bool memory_preload) {
            memory_preload_ = memory_preload;
        }
    protected:
        char* base_;
        size_t mmap_size_;
        bool memory_lock_;
        bool memory_preload_;
    private:
        LOG_DECLARE;
};

}
}