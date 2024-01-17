#!/bin/bash
echo "Compile gazer"
rm -rf build
mkdir build
cd build
cmake .. && cmake --build .
cd ..