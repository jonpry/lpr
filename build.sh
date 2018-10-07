#!/bin/sh
cd pru0
make clean
make
cd ../pru1
make clean
make
cd ..
gcc user.c -o user
