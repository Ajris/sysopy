#!/usr/bin/env bash
./main 1 block imageFile.pgm filter3.txt outFile.pgm >> Times.txt
./main 2 block imageFile.pgm filter3.txt outFile.pgm >> Times.txt
./main 4 block imageFile.pgm filter3.txt outFile.pgm >> Times.txt
./main 8 block imageFile.pgm filter3.txt outFile.pgm >> Times.txt

./main 1 block imageFile.pgm filter33.txt outFile.pgm >> Times.txt
./main 2 block imageFile.pgm filter33.txt outFile.pgm >> Times.txt
./main 4 block imageFile.pgm filter33.txt outFile.pgm >> Times.txt
./main 8 block imageFile.pgm filter33.txt outFile.pgm >> Times.txt

