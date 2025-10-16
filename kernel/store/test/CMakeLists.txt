project(store_test)
enable_testing()

add_executable(file_store_test file_store_test.cc)
add_executable(mmap_store_test mmap_store_test.cc)

target_link_libraries(file_store_test gtest gtest_main store)
target_link_libraries(mmap_store_test gtest gtest_main store)

add_test(NAME FileStore COMMAND file_store_test)
add_test(NAME MMapStore COMMAND mmap_store_test)