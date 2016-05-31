#!/bin/bash

if [ ! -d build ]; then
    mkdir build
fi

cd build

cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/generic-gcc-avr.cmake ..
make $@
