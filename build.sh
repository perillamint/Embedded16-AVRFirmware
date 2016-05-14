#!/bin/bash

cd build

cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/generic-gcc-avr.cmake ..
make
