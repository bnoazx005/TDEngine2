cmake_minimum_required (VERSION 3.8)

project (UnixInputContext CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT DEFINED ${TDENGINE2_LIBRARY_NAME})
	set(TDENGINE2_LIBRARY_NAME "TDEngine2")
endif ()

set(UNIX_INPUT_CONTEXT_LIBRARY_NAME "UnixInputContext")


set(UNIX_INPUT_CTX_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CUnixInputCtxPlugin.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CUnixInputContext.h"
	)

set(UNIX_INPUT_CTX_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CUnixInputCtxPlugin.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CUnixInputContext.cpp"
	)

source_group("includes" FILES ${UNIX_INPUT_CTX_HEADERS})
source_group("sources" FILES ${UNIX_INPUT_CTX_SOURCES})

if (UNIX)

	message(STATUS "UNIX system has detected...")

	include(CheckCXXCompilerFlag)

	CHECK_CXX_COMPILER_FLAG("-std=c++1y" COMPILER_SUPPORTS_CXX14)

	if(COMPILER_SUPPORTS_CXX14)
		message(STATUS "C++14 is enabled")

		set(CMAKE_CXX_STANDARD 14)
		set(CMAKE_CXX_STANDARD_REQUIRED ON)
	else()
	  message(ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
	endif()

endif (UNIX)

add_library(${UNIX_INPUT_CONTEXT_LIBRARY_NAME} SHARED ${UNIX_INPUT_CTX_SOURCES} ${UNIX_INPUT_CTX_HEADERS})

SET_TARGET_PROPERTIES(${UNIX_INPUT_CONTEXT_LIBRARY_NAME} PROPERTIES PREFIX "")

#set up TDEngine2 headers
target_include_directories(${UNIX_INPUT_CONTEXT_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../include")

# disable the <libname>_EXPORTS
set_target_properties(${UNIX_INPUT_CONTEXT_LIBRARY_NAME} PROPERTIES DEFINE_SYMBOL "")

# bad decision to create a circular dependency TODO: dettach the plugin from TDEngine2
target_link_libraries(${UNIX_INPUT_CONTEXT_LIBRARY_NAME} PUBLIC ${TDENGINE2_LIBRARY_NAME})