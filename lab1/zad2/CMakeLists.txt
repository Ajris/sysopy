cmake_minimum_required(VERSION 3.5.1)
project(zad2 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall ")

add_library(zad2 STATIC library.c library.h)

add_executable(main main.c)

target_link_libraries(main zad2)

enable_testing()

add_test(test1 main search_directory ~/Desktop/  sabre tmp.txt  search_directory ~/Android/ sabre tmp.txt search_directory ~/ sabre tmp.txt)
add_test(test2 main create_table 1000 save_and_remove fileMEDIUM.txt 1000)
add_test(test3 main create_table 10 save_block fileSMALL.txt save_block fileMEDIUM.txt save_block fileLARGE.txt remove_block 0 remove_block 1 remove_block 2)