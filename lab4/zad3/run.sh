#!/usr/bin/env bash

cmake .
make
./sender &
./catcher