#!/usr/bin/env bash
cd zad2
cmake .
make
./mainStatic search_directory ~/Desktop/  sabre tmp.txt  search_directory ~/Android/ sabre tmp.txt search_directory ~/ sabre tmp.txt
