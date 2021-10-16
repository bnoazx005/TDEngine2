#!/bin/bash

# "The script for building the engine and its toolset with CMake under UNIX platforms" 
echo off

# "Get all submodules"
git submodule update --recursive --init

if [ $? -ne 0 ]; then
	pause
	exit $?
fi


GENERATOR_NAME="Unix Makefiles"


# "Build bullet3 first"

pushd "../TDEngine2/deps/bullet3"
	cmake -G "$GENERATOR_NAME"  -DBUILD_SHARED_LIBS=OFF -DUSE_GRAPHICAL_BENCHMARK=OFF -DCMAKE_GENERATOR_PLATFORM=$2 -DUSE_MSVC_RUNTIME_LIBRARY_DLL=ON -DCMAKE_BUILD_TYPE=$1 . && cmake --build . --config $1

	# \fixme 
	TDE2_USE_INSTALLED_BULLET=ON
	sudo make install
	
	# if [[ $TDE2_INSTALL_BULLET3 ]]; then
	# 	TDE2_USE_INSTALLED_BULLET=ON
	# 	sudo make install
	# else
	# 	TDE2_USE_INSTALLED_BULLET=OFF
	# fi
popd

if [ $? -ne 0 ]; then
	pause
	exit $?
fi

# Install FMOD's libraries
pushd "../TDEngine2/plugins/FmodAudioContext/deps/fmod/lib/unix_libs/"

# sudo dpkg -i fmodstudio.deb

# if [ $? -ne 0 ]; then
# 	pause
# 	goto finalize
# fi

sudo cp api/core/lib/x86_64/* /usr/lib/

if [ $? -ne 0 ]; then
	pause
	exit $?
fi

popd

if [ $? -ne 0 ]; then
	pause
	exit $?
fi


# try to build tde2_introspector utility
pushd "../TDEngine2/tools/Introspector/source/"
	cmake -G "$GENERATOR_NAME" -DCMAKE_BUILD_TYPE=Release -DCMAKE_GENERATOR_PLATFORM=$2 . && cmake --build . --config Release && cmake -E copy "./bin/Release/tde2_introspector" ".."
popd

if [ $? -ne 0 ]; then
	pause
	exit $?
fi


sh ./run_codegeneration.sh


cmake -G "$GENERATOR_NAME" -DUSE_EXTERNAL_BULLET_LIBRARY=$TDE2_USE_INSTALLED_BULLET -DCMAKE_BUILD_TYPE=$1  .. && cmake --build . --config $1

if [ $? -ne 0 ]; then
	pause
	exit $?
fi

exit $?