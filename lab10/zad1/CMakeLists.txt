cmake_minimum_required(VERSION 3.5.1)
project(lab10-zad1 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_executable(client client.c utils.h)
add_executable(server server.c utils.h)

target_link_libraries(client pthread)
target_link_libraries(client rt)
target_link_libraries(server pthread)
target_link_libraries(server rt)