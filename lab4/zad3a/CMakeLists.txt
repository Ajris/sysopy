cmake_minimum_required(VERSION 3.5.1)
project(lab4-zad3a C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall ")

add_executable(sender sender.c)
add_executable(catcher catcher.c)