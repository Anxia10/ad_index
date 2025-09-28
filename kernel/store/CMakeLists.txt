project(store)

message("PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
include_directories(
    ${PROJECT_SOURCE_DIR}
)

set (
  store_source
  file_store.cc
  store.cc
)

add_library(
  file_store
  ${store_source}
)

# add_subdirectory(test)