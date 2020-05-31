cmake_minimum_required (VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall)
add_compile_options(-Werror)
add_compile_options(-O2)

include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/../../../include)
include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/../../vendor/include)

project (test)

add_executable(
  test
  buttons_test.cpp
  keys_test.cpp
  modifiers_test.cpp
  sizeof_test.cpp
  test.cpp
)
