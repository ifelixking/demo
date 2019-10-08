cmake_minimum_required(VERSION 3.14)
project(eventInMultiThread)

set(CMAKE_CXX_STANDARD 14)

add_executable(eventInMultiThread main.cpp)
target_link_libraries(eventInMultiThread X11 pthread)