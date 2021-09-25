#!/bin/bash

# "The script for building the engine and its toolset with CMake under UNIX platforms" 
echo off

# "Get all submodules"
git submodule update --recursive --init

if [ $? -ne 0 ]; then
	pause
	goto finalize
fi


GENERATOR_NAME="Unix Makefiles"


# "Build bullet3 first"

TDE2_USE_INSTALLED_BULLET="OFF"

pushd "../TDEngine2/deps/bullet3"
	cmake -G "$GENERATOR_NAME"  -DBUILD_SHARED_LIBS=OFF -DUSE_GRAPHICAL_BENCHMARK=OFF -DUSE_MSVC_RUNTIME_LIBRARY_DLL=ON -DCMAKE_BUILD_TYPE=$1 . && cmake --build . --config $1

	if [ $TDE2_INSTALL_BULLET3 -eq 1 ]; then
		TDE2_USE_INSTALLED_BULLET="ON"
		sudo make install
	fi
popd

if [ $? -ne 0 ]; then
	pause
	goto finalize
fi


# try to build tde2_introspector utility
pushd "../TDEngine2/tools/Introspector/source/"
	cmake -G "$GENERATOR_NAME" -DCMAKE_BUILD_TYPE=Release . && cmake --build . --config Release && cmake -E copy "./bin/Release/tde2_introspector" ".."
popd

if [ $? -ne 0 ]; then
	pause
	goto finalize
fi


sh ./run_codegeneration.sh


cmake -G "$GENERATOR_NAME" -DUSE_EXTERNAL_BULLET_LIBRARY=$TDE2_USE_INSTALLED_BULLET -DCMAKE_BUILD_TYPE=$1  .. && cmake --build . --config $1

if [ $? -ne 0 ]; then
	pause
	goto finalize
fi

:finalize
exit $?