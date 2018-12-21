#!/bin/bash

if [ ! -d "build/x86" ]; then
	mkdir -p build/x86
fi

PATH=~/opt/android19-x86/bin:$PATH
cd build/x86

if [ ! -f "Makefile" ]; then
	cmake -DCMAKE_TOOLCHAIN_FILE=../../android-x86.cmake -DCMAKE_INSTALL_PREFIX=~/opt/android19-x86/sysroot/usr ../..
fi

make -j4
