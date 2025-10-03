project(common_log)

add_library(common_log STATIC
  log.cc
)

target_link_libraries(common_log log4cpp)