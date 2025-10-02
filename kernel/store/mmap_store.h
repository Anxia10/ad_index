#pragma once
#include "kernel/store/file_store.h"

namespace kernel {
namespace store {

class MMapStore : public FileStore {
    public:
        MMapStore();
        virtual ~MMapStore();

        data::Status Open(const std::string& name, bool read_only) override;
        data::Status Close() override;
        data::Status Read(const data::Addr& addr, size_t len, data::Data* data) override;
        data::Status Write(const data::Addr& addr, const data::Data& data) override;
    protected:
        char* base_;
        size_t mmap_size_;
        bool memory_lock_;
        bool memory_preload_;

};

}
}