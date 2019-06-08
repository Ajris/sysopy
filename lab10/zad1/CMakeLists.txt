cmake_minimum_required(VERSION 3.5.1)
project(lab10-zad1 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_executable(client client.c common.h)
add_executable(server server.c common.h)

target_link_libraries(server pthread)
target_link_libraries(client pthread)