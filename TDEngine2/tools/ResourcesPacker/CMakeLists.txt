cmake_minimum_required (VERSION 3.8)

project (TDE2ResourcesPacker CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIGURATION>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIGURATION>")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(TDE2_RESOURCES_PACKER_NAME "tde2_resources_packer")


set(UTILITY_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/resourcePacker.h"
	)

set(UTILITY_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/deps/argparse/argparse.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/resourcePacker.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
	)

source_group("includes" FILES ${UTILITY_HEADERS})
source_group("sources" FILES ${UTILITY_SOURCES})


if (MSVC) 	#cl.exe compiler's options
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING)

	#Debug compiler's options
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /std:c++14 /MDd /W3 /GS /Zc:inline /Od /Zi  /Zc:wchar_t /wd4250")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /RTC1 /Gd /Oy- /EHsc /nologo /diagnostics:classic /errorReport:prompt /sdl /permissive- /analyze-")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D _DEBUG")

	#Release compiler's options
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /std:c++14 /permissive- /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /sdl /Zc:inline /wd4250")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /fp:precise /D _WINDLL /D _MBCS /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MD /EHsc /nologo /diagnostics:classic")

endif(MSVC)


if (UNIX)
	message(STATUS "UNIX system has detected...")

	include(CheckCXXCompilerFlag)

	CHECK_CXX_COMPILER_FLAG("-std=c++1z" COMPILER_SUPPORTS_CXX17)

	if(COMPILER_SUPPORTS_CXX17)
		message(STATUS "C++17 is enabled")

		set(CMAKE_CXX_STANDARD 17)
		set(CMAKE_CXX_STANDARD_REQUIRED ON)
	else()
		message(ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
	endif()
endif (UNIX)


add_executable(${TDE2_RESOURCES_PACKER_NAME} ${UTILITY_SOURCES} ${UTILITY_HEADERS})

target_include_directories(${TDE2_RESOURCES_PACKER_NAME} PUBLIC ${ZLIB_INCLUDE_DIRS})
target_link_libraries(${TDE2_RESOURCES_PACKER_NAME} PUBLIC ${ZLIB_LIBRARIES})

if (UNIX)
	target_link_libraries(${TDE2_RESOURCES_PACKER_NAME} pthread stdc++fs)
endif()