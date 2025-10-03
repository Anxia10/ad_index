#include "kernel/common/log/log.h"

namespace kernel {
namespace logging {

int32_t LogGuard::count_ = 0;

LogGuard::LogGuard() {
}

LogGuard::~LogGuard() {
}

void LogGuard::LogConfig(const std::string& config_file) {
    if (count_ == 0) {
        log4cpp::PropertyConfigurator::configure(config_file);
    }
    count_++;
}

void LogGuard::Release() {
    if (count_ == 0) return;
    count_--;
    if (count_ == 0) {
        log4cpp::Category::shutdown();
    }
}

}
}