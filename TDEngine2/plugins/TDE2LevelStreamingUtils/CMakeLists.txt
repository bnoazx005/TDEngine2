cmake_minimum_required (VERSION 3.8)

project (TDE2LevelStreamingUtils CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT DEFINED ${TDENGINE2_LIBRARY_NAME})
	set(TDENGINE2_LIBRARY_NAME "TDEngine2")
endif ()

set(TDE2_LEVEL_STREAMING_LIBRARY_NAME "TDE2LevelStreamingUtils")


set(TDE2_LEVEL_STREAMING_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CLevelStreamingUtilsPlugin.h"
	)

set(TDE2_LEVEL_STREAMING_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CLevelStreamingUtilsPlugin.cpp"
	)

source_group("includes" FILES ${TDE2_LEVEL_STREAMING_HEADERS})
source_group("sources" FILES ${TDE2_LEVEL_STREAMING_SOURCES})


if (MSVC) 	#cl.exe compiler's options
	set(CMAKE_CXX_STANDARD 14)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)

	#Debug compiler's options
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /std:c++14 /MDd /W3 /GS /Zc:inline /Od /Zi  /Zc:wchar_t")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /RTC1 /Gd /Oy- /EHsc /nologo /diagnostics:classic /errorReport:prompt /sdl /permissive- /analyze-  /wd4250")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D _DEBUG")

	#Release compiler's options
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /std:c++14 /permissive- /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /O2 /sdl /Zc:inline  /wd4250")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /fp:precise /D _WINDLL /D _MBCS /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MD /EHsc /nologo /diagnostics:classic")

endif(MSVC)

add_library(${TDE2_LEVEL_STREAMING_LIBRARY_NAME} SHARED ${TDE2_LEVEL_STREAMING_SOURCES} ${TDE2_LEVEL_STREAMING_HEADERS})

SET_TARGET_PROPERTIES(${TDE2_LEVEL_STREAMING_LIBRARY_NAME} PROPERTIES PREFIX "")

#set up TDEngine2 headers
target_include_directories(${TDE2_LEVEL_STREAMING_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../include")

# disable the <libname>_EXPORTS
set_target_properties(${TDE2_LEVEL_STREAMING_LIBRARY_NAME} PROPERTIES DEFINE_SYMBOL "")

# bad decision to create a circular dependency TODO: dettach the plugin from TDEngine2
target_link_libraries(${TDE2_LEVEL_STREAMING_LIBRARY_NAME} PUBLIC ${TDENGINE2_LIBRARY_NAME})