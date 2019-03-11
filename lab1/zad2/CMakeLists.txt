cmake_minimum_required(VERSION 3.5.1)
project(zad2 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall ")

add_library(zad2 STATIC library.c library.h)

add_executable(main main.c)

target_link_libraries(main zad2)