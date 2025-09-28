#pragma once
#include <string>
#include "kernel/common/data/status.h"
#include "kernel/common/data/data.h"

namespace kernel {
namespace store
{


class Store {
public:
    Store();
    virtual ~Store();
    virtual data::Status Open() = 0;
    virtual data::Status Close() = 0;
    virtual data::Status Read() = 0;
    virtual data::Status Write(const data::Addr& addr, const data::Data& data) = 0;

protected:
    std::string name_;
};
}
}