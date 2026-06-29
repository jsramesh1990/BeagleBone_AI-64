# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(hello VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

add_executable(hello hello.c)

install(TARGETS hello DESTINATION bin)
