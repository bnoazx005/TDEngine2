cmake_minimum_required (VERSION 2.8)

project (GLGraphicsContext CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)


set(OGL_GCTX_LIBRARY_NAME "GLGraphicsContext")

# set up GLEW library's directory
link_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../deps/glew-2.1.0/lib")

set(OGL_GCTX_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/win32/CWin32GLContextFactory.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/COGLGraphicsContext.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/IOGLContextFactory.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/COGLGCtxPlugin.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/COGLBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/COGLVertexBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/COGLMappings.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/COGLIndexBuffer.h"
	)

set(OGL_GCTX_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/source/COGLGraphicsContext.cpp"	
	"${CMAKE_CURRENT_SOURCE_DIR}/source/win32/CWin32GLContextFactory.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/COGLGCtxPlugin.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/COGLBuffer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/COGLVertexBuffer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/COGLMappings.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/COGLIndexBuffer.cpp"
	)

source_group("includes" FILES ${OGL_GCTX_HEADERS})
source_group("sources" FILES ${OGL_GCTX_SOURCES})


if (MSVC) 	#cl.exe compiler's options

	#Debug compiler's options
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /std:c++17 /MDd /W3 /GS /Zc:inline /Od /Gm /ZI  /Zc:wchar_t")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /RTC1 /Gd /Oy- /EHsc /nologo /diagnostics:classic /errorReport:prompt /sdl /permissive- /analyze-")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D _DEBUG")

	#Release compiler's options
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /std:c++17 /permissive- /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /sdl /Zc:inline")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /fp:precise /D _WINDLL /D _MBCS /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MD /EHsc /nologo /diagnostics:classic")

endif(MSVC)

if (UNIX)

	message(STATUS "UNIX system has detected...")

	include(CheckCXXCompilerFlag)

	CHECK_CXX_COMPILER_FLAG("-std=c++1y" COMPILER_SUPPORTS_CXX14)

	if(COMPILER_SUPPORTS_CXX14)
	  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
	else()
	  message(ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
	endif()

endif (UNIX)


add_library("${OGL_GCTX_LIBRARY_NAME}" SHARED ${OGL_GCTX_SOURCES} ${OGL_GCTX_HEADERS})

#set up TDEngine2 headers
target_include_directories("${OGL_GCTX_LIBRARY_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../include")

#set up GLEW headers
target_include_directories("${OGL_GCTX_LIBRARY_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../deps/glew-2.1.0/include")


# disable the <libname>_EXPORTS
set_target_properties(${OGL_GCTX_LIBRARY_NAME} PROPERTIES DEFINE_SYMBOL "")