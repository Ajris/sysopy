cmake_minimum_required(VERSION 3.5.1)
project(lab8-zad1 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall ")

add_executable(main main.c info.h)
target_link_libraries(main pthread)
target_link_libraries(main m)