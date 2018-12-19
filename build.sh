#!/bin/sh
cd pru0
make clean
make
cd ../pru1
make clean
make
cd ..
g++ user.cpp file.cpp -o user -lzstd -lpthread -g
