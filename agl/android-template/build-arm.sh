#!/bin/bash

ARCH=arm
TOOLCHAIN=~/opt/android19-$ARCH

mkdir -p build/$ARCH
cd build/$ARCH

PATH=$TOOLCHAIN/bin:$PATH

if [ ! -f "Makefile" ]; then
	cmake \
		-DCMAKE_TOOLCHAIN_FILE=../../android-$ARCH.cmake \
		-DCMAKE_INSTALL_PREFIX=$TOOLCHAIN/sysroot/usr \
		-DARCH=armeabi \
		../..
fi

NPROC=`nproc`
make -j$NPROC
