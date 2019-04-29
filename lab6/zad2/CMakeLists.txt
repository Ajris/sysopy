cmake_minimum_required(VERSION 3.5.1)
project(lab6-zad2 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_executable(client client.c chat.h)
add_executable(server server.c chat.h)

target_link_libraries(server rt)
target_link_libraries(client rt)