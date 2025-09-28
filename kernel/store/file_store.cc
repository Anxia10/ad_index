#include "kernel/store/file_store.h"
namespace kernel {
namespace store {

FileStore::FileStore() {
}

FileStore::~FileStore() {
}

data::Status Open() {
    return data::Status(data::StatusCode::Success);
}

data::Status Close() {
    return data::Status(data::StatusCode::Success);
}

data::Status Read() {
    return data::Status(data::StatusCode::Success);
}

data::Status Write(const data::Addr& addr, const data::Data& data) {
    return data::Status(data::StatusCode::Success);
}


}
}
