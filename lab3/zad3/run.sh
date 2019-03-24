#!/usr/bin/env bash

cmake .
make
./monitor lista.txt 10 1
./tester files/file2 1 2 5