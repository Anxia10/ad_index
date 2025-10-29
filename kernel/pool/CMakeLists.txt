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
add_executable(mmap_pool_test test/mmap_pool_test.cc)
add_executable(fix_length_pool_test test/fix_length_pool_test.cc)
add_executable(variable_length_pool_test test/variable_length_pool_test.cc)

target_link_libraries(file_pool_test gtest gtest_main pool)
target_link_libraries(mmap_pool_test gtest gtest_main pool)
target_link_libraries(fix_length_pool_test gtest gtest_main pool)
target_link_libraries(variable_length_pool_test gtest gtest_main pool)

add_test(NAME FilePool COMMAND file_pool_test)
add_test(NAME MMapPool COMMAND mmap_pool_test)
add_test(NAME FixLengthPool COMMAND fix_length_pool_test)
add_test(NAME VariableLengthPool COMMAND variable_length_pool_test)

