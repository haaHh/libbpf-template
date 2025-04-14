cmake_minimum_required(VERSION 3.10)
project(test)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -ggdb -rdynamic -O0") 

add_executable(test test_main.cc)
