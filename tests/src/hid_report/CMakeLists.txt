cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-Wall)
add_compile_options(-Werror)
add_compile_options(-O2)

include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/../../../include)
include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/../../../vendor/vendor/include)

project (test)

add_executable(
  test
  test.cpp
)
