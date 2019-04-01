#!/usr/bin/env bash

cmake .
make
./catcher KILL
./sender 30 CATCHERPID KILL