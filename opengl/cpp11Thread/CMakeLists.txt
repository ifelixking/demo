cmake_minimum_required(VERSION 3.14)
project(cpp11Thread)

set(CMAKE_CXX_STANDARD 14)

add_executable(cpp11Thread main.cpp)
target_link_libraries(cpp11Thread pthread)