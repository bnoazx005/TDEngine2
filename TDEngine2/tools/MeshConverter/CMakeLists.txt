cmake_minimum_required (VERSION 3.8)

project (TDE2MeshConverter CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(TDE2_MESH_CONVERTER_NAME "tde2_mesh_converter")

if (NOT DEFINED ${TDENGINE2_LIBRARY_NAME})
	set(TDENGINE2_LIBRARY_NAME "TDEngine2")
endif ()

set(UTILITY_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/deps/argparse/argparse.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/deps/Wrench/source/result.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/meshConverter.h"
	)

set(UTILITY_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/deps/argparse/argparse.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/meshConverter.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
	)

source_group("includes" FILES ${UTILITY_HEADERS})
source_group("sources" FILES ${UTILITY_SOURCES})


if (MSVC) 	#cl.exe compiler's options
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)

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


set (ASSIMP_BUILD_OBJ_IMPORTER TRUE)
set (ASSIMP_BUILD_FBX_IMPORTER TRUE)
set (ASSIMP_BUILD_COLLADA_IMPORTER TRUE)

add_subdirectory(deps/assimp)


add_executable(${TDE2_MESH_CONVERTER_NAME} ${UTILITY_SOURCES} ${UTILITY_HEADERS})

if (UNIX)
	target_link_libraries(${TDE2_MESH_CONVERTER_NAME} PUBLIC pthread stdc++fs)
endif()


#set up TDEngine2 headers
target_include_directories(${TDE2_MESH_CONVERTER_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../include")

target_link_libraries(${TDE2_MESH_CONVERTER_NAME} PUBLIC ${TDENGINE2_LIBRARY_NAME})

# set up assimp library and headers
target_include_directories(${TDE2_MESH_CONVERTER_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/deps/assimp/include")

target_link_libraries(${TDE2_MESH_CONVERTER_NAME} PUBLIC assimp)