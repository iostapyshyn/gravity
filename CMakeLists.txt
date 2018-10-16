cmake_minimum_required(VERSION 3.12)
project(gravity C)

set(CMAKE_C_STANDARD 99)

add_executable(gravity gravity.c physics.c physics.h)

find_package(OpenGL REQUIRED)
target_link_libraries(gravity glfw)
target_link_libraries(gravity ${OPENGL_glu_LIBRARY})
