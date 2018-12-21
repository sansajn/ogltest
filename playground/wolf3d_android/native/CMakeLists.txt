cmake_minimum_required(VERSION 3.0)

project(w3dclone)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -llog -landroid -lGLESv2 -lz -lOpenSLES")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/../app/src/main/jniLibs/${ARCH}")

# bullet
include_directories(${SYSROOT}/usr/include/bullet)

include_directories(libs)
include_directories(libs/tools)

add_subdirectory(libs/androidgl)
add_subdirectory(libs/gl)
add_subdirectory(libs/gles2)
add_subdirectory(libs/physics)
add_subdirectory(libs/geometry)
add_subdirectory(libs/al)
add_subdirectory(libs/tools)

set(srcs
	door.cpp
	enemy.cpp
	game.cpp
	level.cpp
	medkit_world.cpp
	medkit.cpp
	player.cpp
	sound.cpp
	resource.cpp
	w3dclone.cpp
)

add_library(w3dclone SHARED ${srcs})

target_link_libraries(w3dclone
	gles2
	gl 
	androidgl 
	al
	tools
	physics
	assimp.a
	BulletDynamics.a 
	BulletCollision.a 
	LinearMath.a
	jpeg.a 
	png12.a
	openal.a
)
