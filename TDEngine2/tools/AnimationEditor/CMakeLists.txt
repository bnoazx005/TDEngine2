cmake_minimum_required (VERSION 3.8)

project (TDE2AnimationEditor CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT DEFINED ${TDENGINE2_LIBRARY_NAME})
	set(TDENGINE2_LIBRARY_NAME "TDEngine2")
endif ()

set(TDE2_ANIMATION_EDITOR_NAME "TDE2AnimationEd")


set(UTILITY_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CUtilityListener.h"
	)

set(UTILITY_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CUtilityListener.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/main.cpp"
	)

source_group("includes" FILES ${UTILITY_HEADERS})
source_group("sources" FILES ${UTILITY_SOURCES})


if (MSVC) 	#cl.exe compiler's options

	#Debug compiler's options
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /std:c++14 /MDd /W3 /GS /Zc:inline /Od /Zi  /Zc:wchar_t /wd4250")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /RTC1 /Gd /Oy- /EHsc /nologo /diagnostics:classic /errorReport:prompt /sdl /permissive- /analyze-")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D _DEBUG")

	#Release compiler's options
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /std:c++14 /permissive- /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /O2 /sdl /Zc:inline /wd4250")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /fp:precise /D _WINDLL /D _MBCS /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MD /EHsc /nologo /diagnostics:classic")

endif(MSVC)


add_executable(${TDE2_ANIMATION_EDITOR_NAME} ${UTILITY_SOURCES} ${UTILITY_HEADERS})

#set up TDEngine2 headers
target_include_directories(${TDE2_ANIMATION_EDITOR_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../include")

#set up GLEW headers
target_include_directories(${TDE2_ANIMATION_EDITOR_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../deps/glew-2.1.0/include")

target_link_libraries(${TDE2_ANIMATION_EDITOR_NAME} PUBLIC ${TDENGINE2_LIBRARY_NAME})