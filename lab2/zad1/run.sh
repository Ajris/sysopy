#!/usr/bin/env bash

cmake .
make
./main generate dane 100 2
./main sort dane 100 2 sys
./main copy dane output 100 2 sys
diff dane output