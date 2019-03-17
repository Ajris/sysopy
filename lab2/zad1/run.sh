#!/usr/bin/env bash

cmake .
make

test(){
    echo "GENERATING FOR SIZE: $1 NUMBER: $2"
    ./main generate dane $1 $2
    cp dane danecp
    echo "SORTING LIB FOR SIZE: $1 NUMBER: $2"
    ./main sort dane $1 $2 lib
    echo "SORTING SYS FOR SIZE: $1 NUMBER: $2"
    ./main sort danecp $1 $2 sys
    echo "COPYING LIB FOR SIZE: $1 NUMBER: $2"
    ./main copy dane output $1 $2 lib
    echo "COPYING SYS FOR SIZE: $1 NUMBER: $2"
    ./main copy danecp outputcp $1 $2 sys
}

test 1 1000
test 4 1000
test 512 1000
test 1024 1000
test 4096 1000
test 8192 1000
