project(common)

add_library(common STATIC
  data/status.cc
  log/log.cc
  concurrency/thread_pool.cc
  concurrency/lock_free_queue.hpp
  utils/utils.cc
)

target_link_libraries(common log4cpp)