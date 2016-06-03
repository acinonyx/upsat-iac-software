cmake_minimum_required(VERSION 2.8)

project(iac C)
add_subdirectory(src)
add_subdirectory(test)
enable_testing()
add_test(
  NAME test
  COMMAND test
  )
