cmake_minimum_required (VERSION 2.8)

project (TDEngine2 CXX)

option(BUILD_D3D11_GCTX_PLUGIN "Build D3D11 Graphics context plugin" ON)
option(BUILD_GL3_0_GCTX_PLUGIN "Build GL3.0 Graphics context plugin" ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin")

if (NOT DEFINED ${TDENGINE2_LIBRARY_NAME})
	set(TDENGINE2_LIBRARY_NAME "TDEngine2")
endif ()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)


set(TDENGINE2_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/TDEngine2.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CBaseObject.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CDefaultEngineCoreBuilder.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CEngineCore.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IBaseObject.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IEngineCore.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IEngineCoreBuilder.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IEngineListener.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IEngineSubsystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IGraphicsContext.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IWindowSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IPlugin.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IDLLManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IFileSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IFile.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CBaseFileSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IResource.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CBaseResource.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IResourceManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IResourceLoader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CResourceManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IResourceHandler.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CResourceHandler.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IJobManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CBaseJobManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IPluginManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CBasePluginManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CBaseComponent.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CComponentManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CEntity.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CEntityManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CSystemManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CWorld.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/IComponent.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/ISystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/ISystemManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/IWorld.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IVertexBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IIndexBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IConstantBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IVertexDeclaration.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/win32/CWin32WindowSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/win32/CWin32Timer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/win32/CWin32DLLManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/win32/CWin32FileSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CTextFileReader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CConfigFileReader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CBaseFile.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/unix/CUnixWindowSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/unix/CUnixDLLManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/unix/CUnixFileSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/unix/CUnixTimer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/CFileLogger.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/Color.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/Config.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/ILogger.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/ITimer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/Types.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/Utils.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/math/TVector3.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/math/TVector4.h"
	)

set(TDENGINE2_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CBaseObject.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CDefaultEngineCoreBuilder.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CEngineCore.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CBaseFileSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CResourceManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CResourceHandler.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CBaseJobManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CBasePluginManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CComponentManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CEntity.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CEntityManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CSystemManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CWorld.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/win32/CWin32WindowSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/win32/CWin32Timer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/win32/CWin32DLLManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/win32/CWin32FileSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/unix/CUnixWindowSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/unix/CUnixDLLManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/unix/CUnixTimer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/unix/CUnixFileSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/CTextFileReader.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/CConfigFileReader.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/utils/CFileLogger.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/utils/Types.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/math/TVector3.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/math/TVector4.cpp")

source_group("includes" FILES ${TDENGINE2_HEADERS})
source_group("sources" FILES ${TDENGINE2_SOURCES})


if (MSVC) 	#cl.exe compiler's options
	#Debug compiler's options
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /std:c++17 /MDd /W3 /GS /Zc:inline /Od /Gm /ZI  /Zc:wchar_t")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /RTC1 /Gd /Oy- /EHsc /nologo /diagnostics:classic /errorReport:prompt /sdl /permissive- /analyze-")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D _DEBUG")

	#Release compiler's options
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /std:c++17 /permissive- /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /sdl /Zc:inline")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /fp:precise /D _WINDLL /D _MBCS /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MD /EHsc /nologo /diagnostics:classic")
endif()

if (UNIX)
	message(STATUS "UNIX system has detected...")

	include(CheckCXXCompilerFlag)

	CHECK_CXX_COMPILER_FLAG("-std=c++1y" COMPILER_SUPPORTS_CXX14)

	if(COMPILER_SUPPORTS_CXX14)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
	else()
		message(ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
	endif()
endif ()


add_library("${TDENGINE2_LIBRARY_NAME}" SHARED ${TDENGINE2_SOURCES} ${TDENGINE2_HEADERS})

if (UNIX)
	# disable lib prefix
	SET_TARGET_PROPERTIES("${TDENGINE2_LIBRARY_NAME}" PROPERTIES PREFIX "")
endif ()

# disable the <libname>_EXPORTS
set_target_properties(${TDENGINE2_LIBRARY_NAME} PROPERTIES DEFINE_SYMBOL "")

#set up GLEW headers
target_include_directories("${TDENGINE2_LIBRARY_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/deps/glew-2.1.0/include")

if (UNIX)
	FIND_PACKAGE(X11 REQUIRED)	# include X11 library

	target_include_directories("${TDENGINE2_LIBRARY_NAME}" PUBLIC "${X11_INCLUDE_DIR}")
	target_link_libraries("${TDENGINE2_LIBRARY_NAME}" PUBLIC "${X11_LIBRARIES}")
endif ()

#build plugins
add_subdirectory(plugins)

if (UNIX)
	install (TARGETS ${TDENGINE2_LIBRARY_NAME} DESTINATION "/usr/lib/TDEngine2")
endif ()