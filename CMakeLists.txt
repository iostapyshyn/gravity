cmake_minimum_required(VERSION 3.12)
project(gravity C)

set(CMAKE_C_STANDARD 99)

add_executable(gravity gravity.c physics.c physics.h gravity.h)
add_definitions(-DGL_SILENCE_DEPRECATION)

find_package(OpenGL REQUIRED)
find_package(glfw3 3.2 REQUIRED)

target_link_libraries(gravity m)
target_link_libraries(gravity pthread)
target_link_libraries(gravity glfw)
target_link_libraries(gravity ${OPENGL_gl_LIBRARY})
