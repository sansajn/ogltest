#!/bin/bash 

if [ ! -d "build/arm" ]; then
	mkdir -p build/arm
fi

PATH=~/opt/android19-arm/bin:$PATH
cd build/arm

if [ ! -f "Makefile" ]; then
	cmake -DCMAKE_TOOLCHAIN_FILE=../../android-arm.cmake -DCMAKE_INSTALL_PREFIX=~/opt/android19-arm/sysroot/usr ../..
fi

make -j4
