cmake_minimum_required (VERSION 3.8)

project (YAMLFormatSupport CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT DEFINED ${TDENGINE2_LIBRARY_NAME})
	set(TDENGINE2_LIBRARY_NAME "TDEngine2")
endif ()

set(YAML_FORMAT_SUPPORT_LIBRARY_NAME "YAMLFormatSupport")


set(YAML_FORMAT_SUPPORT_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CYAMLSupportPlugin.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/IYAMLFile.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CYAMLFile.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/deps/yaml/Yaml.hpp"
	)

set(YAML_FORMAT_SUPPORT_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CYAMLSupportPlugin.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CYAMLFile.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/deps/yaml/Yaml.cpp"
	)

source_group("includes" FILES ${YAML_FORMAT_SUPPORT_HEADERS})
source_group("sources" FILES ${YAML_FORMAT_SUPPORT_SOURCES})


if (MSVC) 	#cl.exe compiler's options

	#Debug compiler's options
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /std:c++17 /MDd /W3 /GS /Zc:inline /Od /Gm /Zi  /Zc:wchar_t")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /RTC1 /Gd /Oy- /EHsc /nologo /diagnostics:classic /errorReport:prompt /sdl /permissive- /analyze-")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D _DEBUG")

	#Release compiler's options
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /std:c++17 /permissive- /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /sdl /Zc:inline")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /fp:precise /D _WINDLL /D _MBCS /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MD /EHsc /nologo /diagnostics:classic")

endif(MSVC)

add_library(${YAML_FORMAT_SUPPORT_LIBRARY_NAME} SHARED ${YAML_FORMAT_SUPPORT_SOURCES} ${YAML_FORMAT_SUPPORT_HEADERS})

#set up TDEngine2 headers
target_include_directories(${YAML_FORMAT_SUPPORT_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../include")
target_include_directories(${YAML_FORMAT_SUPPORT_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/deps/yaml/")

# disable the <libname>_EXPORTS
set_target_properties(${YAML_FORMAT_SUPPORT_LIBRARY_NAME} PROPERTIES DEFINE_SYMBOL "")

# bad decision to create a circular dependency TODO: dettach the plugin from TDEngine2
target_link_libraries(${YAML_FORMAT_SUPPORT_LIBRARY_NAME} PUBLIC ${TDENGINE2_LIBRARY_NAME})