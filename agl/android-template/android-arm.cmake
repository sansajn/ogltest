# spusti prikazom
# 	$ cmake -DCMAKE_TOOLCHAIN_FILE=../android-arm.cmake -DCMAKE_INSTALL_PREFIX=/tmp/android19-arm ..
include(CMakeForceCompiler)
set(CMAKE_SYSTEM_NAME Linux)
set(ARCH armeabi)  # x86, armeabi, mips, ...
CMAKE_FORCE_C_COMPILER(arm-linux-androideabi-gcc GNU)
CMAKE_FORCE_CXX_COMPILER(arm-linux-androideabi-g++ GNU)
