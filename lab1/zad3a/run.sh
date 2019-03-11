#!/usr/bin/env bash

largeSearch="search_directory ~ *.jpg fileLARGE.txt"
mediumSearch="search_directory ~/Desktop/ *.jpg fileMEDIUM.txt"
smallSearch="search_directory ~/Desktop/ *.java fileSMALL.txt"
saveAndRemoveAlternately="create_table 1000 save_and_remove fileMEDIUM.txt 1000"
saveThenRemove="create_table 10 save_block fileSMALL.txt save_block fileMEDIUM.txt save_block fileLARGE.txt remove_block 0 remove_block 1 remove_block 2"

test(){
    $1 ${largeSearch}
    $1 ${mediumSearch}
    $1 ${smallSearch}
    $1 ${saveAndRemoveAlternately}
    $1 ${saveThenRemove}
}

test ./mainStatic
test ./mainDyn
test ./mainShared