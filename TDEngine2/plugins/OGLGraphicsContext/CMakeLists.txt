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
	"${CMAKE_CURRENT_SOURCE_DIR}/include/unix/CUnixGLContextFactory.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/COGLGraphicsContext.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/IOGLContextFactory.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/COGLGCtxPlugin.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/COGLBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/COGLVertexBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/COGLMappings.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/COGLIndexBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/COGLConstantBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/COGLShader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/COGLTexture2D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/COGLShaderCompiler.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/COGLVertexDeclaration.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/COGLGraphicsObjectManager.h"
	)

set(OGL_GCTX_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/source/COGLGraphicsContext.cpp"	
	"${CMAKE_CURRENT_SOURCE_DIR}/source/win32/CWin32GLContextFactory.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/unix/CUnixGLContextFactory.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/COGLGCtxPlugin.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/COGLBuffer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/COGLVertexBuffer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/COGLMappings.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/COGLIndexBuffer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/COGLConstantBuffer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/COGLShader.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/COGLTexture2D.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/COGLShaderCompiler.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/COGLVertexDeclaration.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/COGLGraphicsObjectManager.cpp"
	)

source_group("includes" FILES ${OGL_GCTX_HEADERS})
source_group("sources" FILES ${OGL_GCTX_SOURCES})


if (MSVC) 	#cl.exe compiler's options

	#Debug compiler's options
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /std:c++17 /MDd /W3 /GS /Zc:inline /Od /Gm /Zi  /Zc:wchar_t")
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

SET_TARGET_PROPERTIES("${OGL_GCTX_LIBRARY_NAME}" PROPERTIES PREFIX "")

#set up TDEngine2 headers
target_include_directories("${OGL_GCTX_LIBRARY_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../include")

if (MSVC)
	#set up GLEW headers
	target_include_directories("${OGL_GCTX_LIBRARY_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../deps/glew-2.1.0/include")
endif ()

if (UNIX)
	find_package(GLEW 2.1 REQUIRED)
	
	if (GLEW_FOUND)
	message (STATUS $ENV{GLEW_LOCATION})
		target_include_directories("${OGL_GCTX_LIBRARY_NAME}" PUBLIC ${GLEW_INCLUDE_DIRS})
		set_target_properties("${OGL_GCTX_LIBRARY_NAME}" PROPERTIES LINK_FLAGS "-Wl,-rpath,/usr/lib64/")
		target_link_libraries("${OGL_GCTX_LIBRARY_NAME}" ${GLEW_LIBRARIES})
	endif()
endif ()

# disable the <libname>_EXPORTS
set_target_properties(${OGL_GCTX_LIBRARY_NAME} PROPERTIES DEFINE_SYMBOL "")

if (UNIX)
	install (TARGETS ${OGL_GCTX_LIBRARY_NAME} DESTINATION "/usr/lib/TDEngine2/plugins")
endif ()

# bad decision to create a circular dependency TODO: dettach the plugin from TDEngine2
target_link_libraries(${OGL_GCTX_LIBRARY_NAME} PUBLIC "${TDENGINE2_LIBRARY_NAME}")