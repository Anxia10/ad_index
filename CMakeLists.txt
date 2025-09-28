# 设置最低版本要求
cmake_minimum_required(VERSION 3.5.0)

# 设置项目名称
project(ad_index)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 设置变量
set(store_dir ${PROJECT_SOURCE_DIR}/kernel/store)

include_directories(
    ${PROJECT_SOURCE_DIR}
)

# 编译子文件夹的CmakeLists.txt
add_subdirectory(kernel/store)



