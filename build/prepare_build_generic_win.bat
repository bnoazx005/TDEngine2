echo off

rem "The script for building the engine and its toolset with CMake and Microsoft Visual Studio" 
rem "%1 argument is CMake generator name; %2 is build configuration"


rem "Get all submodules"
git submodule update --recursive --init

if %ERRORLEVEL% neq 0 (
	goto finalize
)


rem "Build bullet3 first"

set TDE2_USE_INSTALLED_BULLET="OFF"

pushd "../TDEngine2/deps/bullet3"
	cmake -G %1 -DBUILD_SHARED_LIBS=OFF -DUSE_GRAPHICAL_BENCHMARK=OFF -DCMAKE_GENERATOR_PLATFORM=%3 -DUSE_MSVC_RUNTIME_LIBRARY_DLL=ON -DCMAKE_BUILD_TYPE=%2 . && cmake --build . --config %2

	if defined TDE2_INSTALL_BULLET3 (
		set TDE2_USE_INSTALLED_BULLET="ON"
		cmake install
	)
popd

if %ERRORLEVEL% neq 0 (
	goto finalize
)


rem "Build zlib"

set TDE2_USE_INSTALLED_ZLIB="OFF"

pushd "../TDEngine2/deps/zlib"
	cmake -G %1 -DCMAKE_BUILD_TYPE=%2 . && cmake --build . --config %2

	if defined TDE2_INSTALL_ZLIB (
		set TDE2_USE_INSTALLED_ZLIB="ON"
		cmake install
	)
popd

if %ERRORLEVEL% neq 0 (
	goto finalize
)


call run_codegeneration.bat


rem "Build main project"

cmake -G %1 -DUSE_EXTERNAL_BULLET_LIBRARY=%TDE2_USE_INSTALLED_BULLET% -DUSE_EXTERNAL_ZLIB_LIBRARY=%TDE2_USE_INSTALLED_ZLIB% -DCMAKE_GENERATOR_PLATFORM=%3 -DCMAKE_BUILD_TYPE=%2 .. && cmake --build . --config %2

exit /b 0

:finalize
exit /b %ERRORLEVEL%