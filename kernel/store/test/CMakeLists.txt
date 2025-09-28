project(store_test)

enable_testing()

add_executable(
  store_test
  file_store_test.cc
)
target_link_libraries(
  store_test
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(store_test)