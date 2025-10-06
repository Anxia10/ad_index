#pragma once
#include <string>
#include "kernel/common/data/status.h"
#include "kernel/common/data/data.h"
#include "kernel/common/log/log.h"

namespace kernel {
namespace store
{


class Store {
    public:
        Store();
        virtual ~Store();
        virtual data::Status Open(const std::string& name, bool read_only) = 0;
        virtual data::Status Close() = 0;
        virtual data::Status Read(const data::Addr& addr, size_t len, data::Data* data) = 0;
        virtual data::Status Write(const data::Addr& addr, const data::Data& data) = 0;
        virtual data::Status Append(const data::Data& data) = 0;
        virtual data::Status Expand(size_t size) = 0;
        virtual data::Status Truncate(size_t size) = 0;
        virtual size_t GetSize() = 0;
        const std::string& GetName() {return name_;}
    protected:
        std::string name_;
    private:
        LOG_DECLARE;

};
}
}