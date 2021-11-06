#!/bin/bash

cd secondary
mkdir -p build
cd build
cmake ..
make 

cd ../../master
mkdir -p build
cd build
cmake ..
make
