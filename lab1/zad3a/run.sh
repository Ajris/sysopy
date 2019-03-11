#!/usr/bin/env bash

largeSearch="search_directory ~ *.jpg fileLARGE.txt"
mediumSearch="search_directory ~/Desktop/ *.jpg fileMEDIUM.txt"
smallSearch="search_directory ~/Desktop/ *.java fileSMALL.txt"
saveAndRemoveAlternately="create_table 10 save_and_remove fileMEDIUM.txt 10"
saveThenRemove="create_table 10 save_block fileSMALL.txt save_block fileMEDIUM.txt save_block fileLARGE.txt remove_block 0 remove_block 1 remove_block 2"

test(){
    echo "$1 largesearch"
    $1 ${largeSearch}
    echo "$1 mediumSearch"
    $1 ${mediumSearch}
        echo "$1 smallSearch"
    $1 ${smallSearch}
        echo "$1 saveAndRemoveAlternately 1 -> allocating 2-> saving and removing"
    $1 ${saveAndRemoveAlternately}
        echo "$1 saveThenRemove 1 -> allocating 2,3,4 -> saving 5,6,7 -> removing"
    $1 ${saveThenRemove}
}

test ./mainStatic
test ./mainDyn
test ./mainShared