#!/bin/bash

# "The script for building the engine and its toolset with CMake under UNIX platforms" 
echo off

# "Get all submodules"
git submodule update --recursive --init

if [$? -ne 0]; then
	pause
	goto finalize
fi


# "Build bullet3 first"

TDE2_USE_INSTALLED_BULLET="OFF"

pushd "../TDEngine2/deps/bullet3"
	cmake -G $1 -DBUILD_SHARED_LIBS=OFF -DUSE_GRAPHICAL_BENCHMARK=OFF -DUSE_MSVC_RUNTIME_LIBRARY_DLL=ON . && cmake --build .

	if [[$TDE2_INSTALL_BULLET3]]; then
		TDE2_USE_INSTALLED_BULLET="ON"
		cmake install
	fi
popd

if [$? -ne 0]; then
	pause
	goto finalize
fi


cmake -G $1 -DUSE_EXTERNAL_BULLET_LIBRARY=$TDE2_USE_INSTALLED_BULLET .. && \
cmake --build .

if [$? -ne 0]; then
	pause
	goto finalize
fi

:finalize
exit $?