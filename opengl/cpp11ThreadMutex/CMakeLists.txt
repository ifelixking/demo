cmake_minimum_required(VERSION 3.14)
project(cpp11ThreadMutex)

set(CMAKE_CXX_STANDARD 14)

add_executable(cpp11ThreadMutex main.cpp)
target_link_libraries(cpp11ThreadMutex pthread)