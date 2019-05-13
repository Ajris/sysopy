cmake_minimum_required(VERSION 3.5.1)
project(lab7-zad2 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall ")

add_executable(loader loader.c both.h both.c)
add_executable(trucker trucker.c both.h both.c)

target_link_libraries(loader rt pthread)
target_link_libraries(trucker rt pthread)