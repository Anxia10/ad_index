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

        const std::string& GetReason() {
            return reason;
        }
    private:
        StatusCode status_code;
        std::string reason;
};

}
}