#pragma once
#include<string>

namespace kernel {
namespace data {

enum StatusCode {
    Success = 0,
    Exception = 1,
    OperatorForbidden = 2  
};

class Status {
    public:
        Status(StatusCode code = Success);
        Status(StatusCode code, const std::string& rs);
        ~Status();

        bool operate() {
            return status_code == StatusCode::Success;
        }
    private:
        StatusCode status_code;
        std::string reason;
};

}
}