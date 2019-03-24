#!/usr/bin/env bash

cmake .
make
./monitor lista.txt 30 1 1 1 &
./tester files/file1 1 2 5