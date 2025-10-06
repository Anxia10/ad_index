project(index)

include_directories(
    ${PROJECT_SOURCE_DIR}
)

add_library(index
  hash_table/hash_bucket.cc
  hash_table/murmurhash_table.cc
)
target_link_libraries(index common_data common_log)
