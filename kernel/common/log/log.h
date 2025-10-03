#pragma once
#include <log4cpp/Category.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/PropertyConfigurator.hh>

#define KDefaultLogConfigFile "log4cpp.properties"
namespace kernel {
namespace logging {

class LogGuard {
    public:
        static void LogConfig(const std::string& config_file);
        static void Release();
        static int32_t GetCount() {return count_;};
    private:
        static int32_t count_;
        LogGuard();
        ~LogGuard();
};
}
}

#define LOG_CONFIG(config_file) kernel::logging::LogGuard::LogConfig(config_file)

#define LOG_SHUTDOWN() kernel::logging::LogGuard::Release()

#define LOG_DECLARE static log4cpp::Category* _logger

#define LOG_SETUP(category, Clazz) log4cpp::Category* Clazz::_logger = &log4cpp::Category::getInstance(category "." #Clazz)

#define LOG_DECLARE_AND_SETUP(category) static log4cpp::Category* _loggeer = &log4cpp::Category::getInstance(category)

#define LOG_DEBUG(format, ...) if (_logger->isDebugEnabled()) _logger->debug("%s:%d#%s " format, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#define LOG_INFO(format, ...) if (_logger->isInfoEnabled()) _logger->info("%s:%d#%s " format, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#define LOG_WARN(format, ...) if (_logger->isWarnEnabled()) _logger->warn("%s:%d#%s " format, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#define LOG_ERROR(format, ...) if (_logger->isErrorEnabled()) _logger->error("%s:%d#%s " format, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)