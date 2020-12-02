echo off
rem "The script for building the engine and its toolset with CMake and Microsoft Visual Studio 2017" 

call prepare_build_generic_win.bat "Visual Studio 15 2017"
exit /b %ERRORLEVEL%