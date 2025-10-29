project(common)

add_library(common STATIC
  data/status.cc
  log/log.cc
  concurrency/thread_pool.cc
  concurrency/lock_free_queue.hpp
  utils/utils.cc
)

target_link_libraries(common log4cpp)

# 测试
enable_testing()

add_executable(lock_free_queue_test common_test/lock_free_queue_test.cc)
add_executable(thread_pool_test common_test/thread_pool_test.cc)

target_link_libraries(lock_free_queue_test gtest gtest_main index)
target_link_libraries(thread_pool_test gtest gtest_main index)

add_test(NAME LockFreeQueue COMMAND lock_free_queue_test)
add_test(NAME ThreadPool COMMAND thread_pool_test)