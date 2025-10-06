project(store)

include_directories(
    ${PROJECT_SOURCE_DIR}
)

add_library(store
  file_store.cc
  store.cc
)
target_link_libraries(store common_data common_log)

add_subdirectory(test)