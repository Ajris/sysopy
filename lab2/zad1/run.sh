#!/usr/bin/env bash

cmake .
make
./main generate dane 100 2
./main sort dane 100 2 lib
./main copy dane output 100 2 lib