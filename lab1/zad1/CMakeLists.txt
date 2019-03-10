cmake_minimum_required(VERSION 3.5.1)
project(zad1 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall ")

add_library(zad1Shared SHARED library.c library.h)
add_library(zad1Static STATIC library.c library.h)