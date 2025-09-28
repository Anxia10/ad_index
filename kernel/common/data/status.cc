#include "kernel/common/data/status.h"

namespace kernel {
namespace data {

Status::Status(StatusCode code) :
    status_code(code) {
}

Status::Status(StatusCode code, const std::string& rs) :
    status_code(code),
    reason(rs){
}

Status::~Status() {
}

}
}