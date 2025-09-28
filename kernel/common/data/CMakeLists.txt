project(common_data)

set (
  common_data_source
  status.cc
)

add_executable(
  common_data
  ${common_data_source}
)