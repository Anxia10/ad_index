#pragma once
#include <cstdint>
#include "kernel/store/store.h"

namespace kernel {
namespace store {

class FileStore : public Store {
    public:
        FileStore();
        virtual ~FileStore();

        data::Status Open(const std::string& name, bool read_only = false) override;
        data::Status Close() override;
        data::Status Read(const data::Addr& addr, size_t len, data::Data* data) override;
        data::Status Write(const data::Addr& addr, const data::Data& data) override;
        data::Status Append(const data::Data& data) override;
        
        size_t GetSize() override {
            return size_;
        }

        const std::string& GetName() {
            return name_;
        }
    private:
        size_t GetFileSize();
        LOG_DECLARE;
    protected:
        int32_t fd_;
        size_t size_;
};

}
}