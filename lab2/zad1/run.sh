#!/usr/bin/env bash

cmake .
make
./main generate dane 100 512
#./main sort
#./main copy