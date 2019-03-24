cmake_minimum_required(VERSION 3.5.1)
project(lab3-zad3 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall ")

add_executable(monitor main.c)
add_executable(tester tester.c)
