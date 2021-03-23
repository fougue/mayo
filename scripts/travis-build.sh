#!/bin/bash

# Make the script fails on any command error
set -e

# Create out-of-source dir
mkdir build
cd build

# Run QMake
qmake --version
qmake ../mayo.pro CASCADE_INC_DIR=/usr/include/opencascade

# Make
make -j2
