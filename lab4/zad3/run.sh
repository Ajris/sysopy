#!/usr/bin/env bash

cmake .
make
./sender 30 &
./catcher