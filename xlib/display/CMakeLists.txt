cmake_minimum_required(VERSION 3.14)
project(display)

set(CMAKE_CXX_STANDARD 14)

add_executable(display main.cpp)
target_link_libraries(display X11 pthread)