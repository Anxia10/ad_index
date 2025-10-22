project(index)

include_directories(
    ${PROJECT_SOURCE_DIR}
)

add_library(index
  index.cc
  hash_table/hash_bucket.cc
  hash_table/murmurhash_table.cc
  sort_table/sort_table.cc
)
target_link_libraries(index common)
target_link_libraries(index store)
target_link_libraries(index pool)

# 测试
enable_testing()

add_executable(murmurhash_table_test hash_table/test/murmurhash_table_test.cc)
add_executable(hash_bucket_test hash_table/test/hash_bucket_test.cc)

target_link_libraries(murmurhash_table_test gtest gtest_main index)
target_link_libraries(hash_bucket_test gtest gtest_main index)

add_test(NAME MurMurHashTable COMMAND murmurhash_table_test)
add_test(NAME HashBucket COMMAND hash_bucket_test)