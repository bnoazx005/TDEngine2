#!/bin/bash

# "The script for building the engine and its toolset with CMake under UNIX platforms" 
echo off


function pause {
	echo "Press any key to continue..."
	read
}


# "Get all submodules"
git submodule update --recursive --init

if [ $? -ne 0 ]; then
	pause
	exit $?
fi


GENERATOR_NAME="Unix Makefiles"


# "Build bullet3 first"

pushd "../TDEngine2/plugins/TDE2BulletPhysics/deps/bullet3"
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


# "Build zlib first"

pushd "../TDEngine2/deps/zlib"
	cmake -G "$GENERATOR_NAME" -DCMAKE_GENERATOR_PLATFORM=$2 -DCMAKE_BUILD_TYPE=$1 . && cmake --build . --config $1

	# \fixme 
	TDE2_USE_INSTALLED_ZLIB=ON
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
#pushd "../TDEngine2/plugins/FmodAudioContext/deps/fmod/lib/linux/"

# sudo dpkg -i fmodstudio.deb

# if [ $? -ne 0 ]; then
# 	pause
# 	goto finalize
# fi

#tar -xzf fmodstudioapi20206linux.tar.gz

#sudo cp ./fmodstudioapi20206linux/api/core/lib/x86_64/*so /usr/local/lib/
#sudo cp ./fmodstudioapi20206linux/api/studio/lib/x86_64/*so /usr/local/lib/
#sudo cp ./fmodstudioapi20206linux/api/fsbank/lib/x86_64/*so /usr/local/lib/

#rm -r fmodstudioapi20206linux
#rm fmodstudioapi20206linux.tar.gz

#if [ $? -ne 0 ]; then
#	pause
#	exit $?
#fi

#popd

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


cmake -G "$GENERATOR_NAME" -DASSIMP_BUILD_ASSIMP_TOOLS=OFF -DBUILD_FMOD_AUDIO_CTX_PLUGIN=OFF -DUSE_EXTERNAL_BULLET_LIBRARY=$TDE2_USE_INSTALLED_BULLET -DUSE_EXTERNAL_ZLIB_LIBRARY=$TDE2_USE_INSTALLED_ZLIB -DCMAKE_BUILD_TYPE=$1  .. && cmake --build . --config $1

if [ $? -ne 0 ]; then
	pause
	exit $?
fi

exit $?