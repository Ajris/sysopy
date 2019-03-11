cmake_minimum_required(VERSION 3.5.1)
project(zad3b C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c11 -fPIC -Wl,-rpath=./ -ldl")


foreach (loop_var -O -O1 -O2 -O3 -Os)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${loop_var}")
    add_library(zad3bShared${loop_var} SHARED library.c library.h)
    add_library(zad3bStatic${loop_var} STATIC library.c library.h)
    add_library(zad3bDynamic${loop_var} MODULE library.c library.h)

    add_executable(mainStatic${loop_var} main.c)
    add_executable(mainShared${loop_var} main.c)
    add_executable(mainDyn${loop_var} mainLibDynamic.c)

    target_link_libraries(mainShared${loop_var} zad3bShared${loop_var})
    target_link_libraries(mainStatic${loop_var} zad3bStatic${loop_var})
    target_link_libraries(mainDyn${loop_var} ${CMAKE_DL_LIBS})

    enable_testing()

    add_test(testStat1 mainStatic search_directory ~/Desktop/ sabre tmp.txt search_directory ~/Android/ sabre tmp.txt search_directory ~/ sabre tmp.txt)
    add_test(testStat2 mainStatic create_table 1000 save_and_remove fileMEDIUM.txt 1000)
    add_test(testStat3 mainStatic create_table 10 save_block fileSMALL.txt save_block fileMEDIUM.txt save_block fileLARGE.txt remove_block 0 remove_block 1 remove_block 2)

    add_test(testShar1 mainShared search_directory ~/Desktop/ sabre tmp.txt search_directory ~/Android/ sabre tmp.txt search_directory ~/ sabre tmp.txt)
    add_test(testShar2 mainShared create_table 1000 save_and_remove fileMEDIUM.txt 1000)
    add_test(testShar3 mainShared create_table 10 save_block fileSMALL.txt save_block fileMEDIUM.txt save_block fileLARGE.txt remove_block 0 remove_block 1 remove_block 2)

    add_test(testDyn1 mainDyn search_directory ~/Desktop/ sabre tmp.txt search_directory ~/Android/ sabre tmp.txt search_directory ~/ sabre tmp.txt)
    add_test(testDyn2 mainDyn create_table 1000 save_and_remove fileMEDIUM.txt 1000)
    add_test(testDyn3 mainDyn create_table 10 save_block fileSMALL.txt save_block fileMEDIUM.txt save_block fileLARGE.txt remove_block 0 remove_block 1 remove_block 2)
endforeach (loop_var)