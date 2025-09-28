#pragma once
#include <cstdint>
#include "kernel/store/store.h"

namespace kernel {
namespace store {

class FileStore : public Store {
    public:
        FileStore();
        virtual ~FileStore();

        data::Status Open() override;
        data::Status Close() override;
        data::Status Read() override;
        data::Status Write(const data::Addr& addr, const data::Data& data) override;

        const std::string& GetName() {
            return name_;
        }
    protected:
        int32_t fd_;
        size_t size_;
};

}
}