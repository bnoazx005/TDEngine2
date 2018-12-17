cmake_minimum_required (VERSION 2.8)

project (WindowsInputContext CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)


set(WININPUT_CONTEXT_LIBRARY_NAME "WindowsInputContext")


set(WIN_INPUT_CTX_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CWindowsInputCtxPlugin.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CWindowsInputContext.h"	
	"${CMAKE_CURRENT_SOURCE_DIR}/include/IInputDevice.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CBaseInputDevice.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CKeyboard.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CMouse.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CDirectInputMappings.h"
	)

set(WIN_INPUT_CTX_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CWindowsInputCtxPlugin.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CWindowsInputContext.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CBaseInputDevice.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CKeyboard.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CMouse.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CDirectInputMappings.cpp"
	)

source_group("includes" FILES ${WIN_INPUT_CTX_HEADERS})
source_group("sources" FILES ${WIN_INPUT_CTX_SOURCES})


if (MSVC) 	#cl.exe compiler's options

	#Debug compiler's options
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /std:c++17 /MDd /W3 /GS /Zc:inline /Od /Gm /Zi  /Zc:wchar_t")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /RTC1 /Gd /Oy- /EHsc /nologo /diagnostics:classic /errorReport:prompt /sdl /permissive- /analyze-")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D _DEBUG")

	#Release compiler's options
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /std:c++17 /permissive- /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /sdl /Zc:inline")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /fp:precise /D _WINDLL /D _MBCS /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MD /EHsc /nologo /diagnostics:classic")

endif(MSVC)

add_library("${WININPUT_CONTEXT_LIBRARY_NAME}" SHARED ${WIN_INPUT_CTX_SOURCES} ${WIN_INPUT_CTX_HEADERS})

#set up TDEngine2 headers
target_include_directories("${WININPUT_CONTEXT_LIBRARY_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../include")

# disable the <libname>_EXPORTS
set_target_properties(${WININPUT_CONTEXT_LIBRARY_NAME} PROPERTIES DEFINE_SYMBOL "")

# bad decision to create a circular dependency TODO: dettach the plugin from TDEngine2
target_link_libraries(${WININPUT_CONTEXT_LIBRARY_NAME} PUBLIC "${TDENGINE2_LIBRARY_NAME}")