cmake_minimum_required (VERSION 3.8)

project (ImGUIContext CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT DEFINED ${TDENGINE2_LIBRARY_NAME})
	set(TDENGINE2_LIBRARY_NAME "TDEngine2")
endif ()

set(IMGUI_CONTEXT_LIBRARY_NAME "ImGUIContext")


set(IMGUI_CTX_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CImGUICtxPlugin.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CImGUIContext.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../deps/imgui-1.72/imgui.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../deps/imgui-1.72/imgui_internal.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../deps/imgui-1.72/ImGuizmo.h"
	)

set(IMGUI_CTX_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CImGUICtxPlugin.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CImGUIContext.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../deps/imgui-1.72/imgui.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../deps/imgui-1.72/imgui_widgets.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../deps/imgui-1.72/imgui_draw.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../deps/imgui-1.72/imgui_demo.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../deps/imgui-1.72/ImGuizmo.cpp"
	)

source_group("includes" FILES ${IMGUI_CTX_HEADERS})
source_group("sources" FILES ${IMGUI_CTX_SOURCES})


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

add_library(${IMGUI_CONTEXT_LIBRARY_NAME} SHARED ${IMGUI_CTX_SOURCES} ${IMGUI_CTX_HEADERS})

#set up TDEngine2 headers
target_include_directories(${IMGUI_CONTEXT_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../include")
#set up imGUI headers
target_include_directories(${IMGUI_CONTEXT_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../deps/imgui-1.72/")

# disable the <libname>_EXPORTS
set_target_properties(${IMGUI_CONTEXT_LIBRARY_NAME} PROPERTIES DEFINE_SYMBOL "")

# bad decision to create a circular dependency TODO: dettach the plugin from TDEngine2
target_link_libraries(${IMGUI_CONTEXT_LIBRARY_NAME} PUBLIC ${TDENGINE2_LIBRARY_NAME})