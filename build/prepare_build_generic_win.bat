rem "The script for building the engine and its toolset with CMake and Microsoft Visual Studio" 
rem "%1 argument is CMake generator name; %2 is extra build configuration"
echo off

rem "Get all submodules"
git submodule update --recursive --init

if %ERRORLEVEL% neq 0 (
	goto finalize
)


rem "Build bullet3 first"

set TDE2_USE_INSTALLED_BULLET="OFF"

pushd "../TDEngine2/deps/bullet3"
	cmake -G %1 -DBUILD_SHARED_LIBS=OFF -DUSE_GRAPHICAL_BENCHMARK=OFF -DUSE_MSVC_RUNTIME_LIBRARY_DLL=ON . && cmake --build .

	if defined TDE2_INSTALL_BULLET3 (
		set TDE2_USE_INSTALLED_BULLET="ON"
		cmake install
	)
popd

if %ERRORLEVEL% neq 0 (
	goto finalize
)


call run_codegeneration.bat


rem "Build main project"

cmake -G %1 -DUSE_EXTERNAL_BULLET_LIBRARY=%TDE2_USE_INSTALLED_BULLET% .. && \
cmake --build . %2

if %ERRORLEVEL% neq 0 (
	pause
)

:finalize
exit /b %ERRORLEVEL%