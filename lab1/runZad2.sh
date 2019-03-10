#!/bin/bash
CT="create_table 10"
ST="search_directory CMakeFiles mainStatic fileSMALL.txt"
SB="save_block fileMEDIUM.txt"
RB="remove_block"
searchSmall="search_directory ~/Desktop/  sabre tmp.txt"
searchMedium="search_directory ~/Android/ sabre tmp.txt"
searchLarge="search_directory ~/ sabre tmp.txt"
saveSmall="save_block fileSMALL.txt"
saveMedium="save_block fileMEDIUM.txt"
saveLarge="save_block fileLARGE.txt"
ALL="${CT} ${searchSmall} ${searchMedium} ${searchLarge} ${saveSmall} ${saveMedium} ${saveLarge} ${RB} 0 ${RB} 1 ${RB} 2"
for i in `seq 1 10`;
do
    ALL="${ALL} ${SB} ${RB} 0"
done

cd zad2
cmake .
make
./mainStatic ${ALL}

