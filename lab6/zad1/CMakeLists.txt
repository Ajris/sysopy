cmake_minimum_required(VERSION 3.5.1)
project(lab6-zad1 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall ")

add_executable(client client.c)
add_executable(server server.c)