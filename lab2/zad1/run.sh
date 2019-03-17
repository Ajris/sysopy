#!/usr/bin/env bash

cmake .
make
./main generate dane 100 2
./main sort dane 100 2 lib
head dane
#./main copy