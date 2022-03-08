echo off
rem "The script for building the engine and its toolset with CMake and Microsoft Visual Studio 2022" 

call prepare_build_generic_win.bat "Visual Studio 17 2022" Debug x64
exit /b %ERRORLEVEL%