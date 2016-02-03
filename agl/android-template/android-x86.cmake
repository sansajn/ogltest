# spusti prikazom
# 	$ cmake -DCMAKE_TOOLCHAIN_FILE=../android-x86.cmake -DCMAKE_INSTALL_PREFIX=/tmp/android19-x86 ..
include(CMakeForceCompiler)
set(CMAKE_SYSTEM_NAME Linux)
CMAKE_FORCE_C_COMPILER(i686-linux-android-gcc GNU)
CMAKE_FORCE_CXX_COMPILER(i686-linux-android-g++ GNU)
