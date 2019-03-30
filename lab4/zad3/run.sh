#!/usr/bin/env bash

cmake .
make
./sender.c &
./catcher.c