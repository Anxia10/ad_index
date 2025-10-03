#include "kernel/test/test.h"
#include <iostream>
#include <log4cpp/Category.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/SimpleLayout.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/Priority.hh>

Test::Test() {

}

Test::~Test() {
    
}

void InitParament() {
}

int main()
{
    std::cout << "Starting log4cpp test..." << std::endl;
        
        // 1. 创建控制台输出
        log4cpp::Appender* consoleAppender = new log4cpp::OstreamAppender("console", &std::cout);
        log4cpp::PatternLayout* consoleLayout = new log4cpp::PatternLayout();
        consoleLayout->setConversionPattern("%d [%p] %c: %m%n");
        consoleAppender->setLayout(consoleLayout);
        
        // 2. 创建文件输出
        log4cpp::Appender* fileAppender = new log4cpp::FileAppender("file", "test.log");
        log4cpp::PatternLayout* fileLayout = new log4cpp::PatternLayout();
        fileLayout->setConversionPattern("%d [%p] %c: %m%n");
        fileAppender->setLayout(fileLayout);
        
        // 3. 获取根分类并添加输出器
        log4cpp::Category& root = log4cpp::Category::getRoot();
        root.setAppender(consoleAppender);
        root.setAppender(fileAppender);
        root.setPriority(log4cpp::Priority::DEBUG);
        
        // 4. 获取子分类
        log4cpp::Category& sub1 = log4cpp::Category::getInstance("sub1");
        log4cpp::Category& sub2 = log4cpp::Category::getInstance("sub2");
        
        std::cout << "Logging test messages..." << std::endl;
        
        // 5. 测试各种日志级别
        root.debug("This is a DEBUG message from root");
        root.info("This is an INFO message from root");
        root.notice("This is a NOTICE message from root");
        root.warn("This is a WARN message from root");
        root.error("This is an ERROR message from root");
        root.crit("This is a CRITICAL message from root");
        root.alert("This is an ALERT message from root");
        root.fatal("This is a FATAL message from root");
        
        sub1.info("This is an INFO message from sub1");
        sub2.warn("This is a WARN message from sub2");
        
        // 6. 测试格式化输出
        int value = 42;
        const char* name = "test";
        root.info("Formatted message: value=%d, name=%s", value, name);
        
        std::cout << "All log messages sent successfully!" << std::endl;
        std::cout << "Check test.log file for file output" << std::endl;
        
        // 7. 清理
        log4cpp::Category::shutdown();
}