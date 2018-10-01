@echo off

:loop

if "%1" == "" goto :finalize
if /i "%1" == "msvc14" goto :msvc14
if /i "%1" == "msvc15" goto :msvc15
if /i "%1" == "x86" goto :x86
if /i "%1" == "amd64" goto :amd64
if /i "%1" == "x86_64" goto :amd64
if /i "%1" == "x64" goto :amd64

echo Invalid argument: '%1'
exit -1

:msvc14
set TOOLCHAIN=msvc14
set CMAKE_GENERATOR=Visual Studio 14 2015
shift
goto :loop

:msvc15
set TOOLCHAIN=msvc15
set CMAKE_GENERATOR=Visual Studio 15 2017
shift
goto :loop

:x86
set TARGET_CPU=x86
set CMAKE_GENERATOR_SUFFIX=
shift
goto :loop

:amd64
set TARGET_CPU=amd64
set CMAKE_GENERATOR_SUFFIX= Win64
shift
goto :loop

:finalize

if "%TOOLCHAIN%" == "" goto :msvc14
if "%TARGET_CPU%" == "" goto :amd64
if "%CONFIGURATION%" == "" (set CONFIGURATION=Release)

set CMAKE_CONFIGURE_FLAGS=-G "%CMAKE_GENERATOR%%CMAKE_GENERATOR_SUFFIX%"

set CMAKE_BUILD_FLAGS= ^
    --config %CONFIGURATION% ^
    -- ^
    /nologo ^
    /verbosity:minimal ^
    /consoleloggerparameters:Summary