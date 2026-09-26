cmake_minimum_required(VERSION 3.24 FATAL_ERROR)
project(constructor_exception_test)

set(CMAKE_CXX_STANDARD 23)
add_compile_options(-Wall -Werror -O2 -fsanitize=address)
add_link_options(-fsanitize=address)

include_directories(${CMAKE_CURRENT_LIST_DIR}/../../../include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../../../src/Daemon/include)
include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/../../../vendor/vendor/include)

add_executable(test test.cpp)
target_link_libraries(test "-framework CoreFoundation" "-framework IOKit")
