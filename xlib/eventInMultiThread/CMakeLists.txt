cmake_minimum_required(VERSION 3.14)
project(eventInMultiThread)

set(CMAKE_CXX_STANDARD 14)

if(${APPLE})
    include_directories(/opt/X11/include)
    link_directories(/opt/X11/lib)
    add_compile_options(-Wno-deprecated-register)
endif()

add_executable(eventInMultiThread main.cpp)
target_link_libraries(eventInMultiThread X11 pthread)