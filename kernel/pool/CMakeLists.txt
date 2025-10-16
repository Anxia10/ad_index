project(pool)

include_directories(
    ${PROJECT_SOURCE_DIR}
)

add_library(pool
  pool.cc
  file_pool.cc
  mmap_pool.cc
  fix_length_pool.cc
  variable_length_pool.cc
)
target_link_libraries(pool common)
target_link_libraries(pool store)

# 测试
enable_testing()

add_executable(file_pool_test test/file_pool_test.cc)

target_link_libraries(file_pool_test gtest gtest_main pool)

add_test(NAME FilePool COMMAND file_pool_test)
