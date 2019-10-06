cmake_minimum_required(VERSION 3.15)
project(multiThread)

set(CMAKE_CXX_STANDARD 14)


# FIND OPENGL
#########################################################
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)

add_executable(multiThread main.cpp)
target_link_libraries(multiThread GLEW ${OPENGL_LIBRARIES} X11 GLX GL pthread)