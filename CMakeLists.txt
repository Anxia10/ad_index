# 设置最低版本要求
cmake_minimum_required(VERSION 3.5.0)

# 设置项目名称
project(ad_index)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 设置变量
set(store_dir ${PROJECT_SOURCE_DIR}/kernel/store)
set(test_data_dir ${PROJECT_SOURCE_DIR}/kernel/test/testdata)
set(LOG4CPP_LIBRARIES log4cpp CACHE INTERNAL "log4cpp library")

# 首先创建主库目标（空的接口库）
add_library(ad_index INTERFACE)

# 为主库目标设置属性
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${LOG4CPP_INCLUDE_DIRS}
)

include(FetchContent)
FetchContent_Declare(
  rapidjson
  URL https://github.com/Tencent/rapidjson/archive/refs/tags/v1.1.0.zip
)
FetchContent_MakeAvailable(rapidjson)

# GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)



add_definitions(-Dtest_data_dir="${test_data_dir}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options("-mavx2")
# 编译子文件夹
add_subdirectory(kernel/common)
add_subdirectory(kernel/store)
add_subdirectory(kernel/index)
add_subdirectory(kernel/test)
add_subdirectory(kernel/pool)
