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
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IResourceFactory.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/memory/IAllocator.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/memory/CBaseAllocator.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/memory/CLinearAllocator.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/memory/CStackAllocator.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/memory/CPoolAllocator.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/memory/IMemoryManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/memory/CMemoryManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/Event.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IEventManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CEventManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IInputContext.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IInputDevice.h"
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
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/ITransform.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CTransform.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/IComponentFactory.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/IComponentManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CTransformSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CSpriteRendererSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IVertexBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IIndexBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IConstantBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IVertexDeclaration.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IShader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CBaseShader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IShaderCompiler.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CBaseShaderLoader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/ISprite.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CQuadSprite.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/ITexture.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/ITexture2D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/ITextureCube.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CBaseTexture2D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/ICamera.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CBaseCamera.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CPerspectiveCamera.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/COrthoCamera.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CBaseShaderCompiler.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CVertexDeclaration.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/InternalShaderData.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IGraphicsObjectManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CBaseGraphicsObjectManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IRenderTarget.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CBaseRenderTarget.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IRenderable.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IRenderer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CForwardRenderer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CRenderQueue.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IGraphicsLayersInfo.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CGraphicsLayersInfo.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IMaterial.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CBaseMaterial.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/win32/CWin32WindowSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/win32/CWin32Timer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/win32/CWin32DLLManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/win32/CWin32FileSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CTextFileReader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CConfigFileReader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CBaseFile.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CBinaryFileReader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CBinaryFileWriter.h"
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
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/CResult.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/math/TVector2.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/math/TVector3.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/math/TVector4.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/math/TMatrix3.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/math/TMatrix4.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/math/TQuaternion.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/math/TRect.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/math/MathUtils.h"
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
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CBaseResource.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/memory/CBaseAllocator.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/memory/CLinearAllocator.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/memory/CStackAllocator.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/memory/CPoolAllocator.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/memory/CMemoryManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CEventManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CComponentManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CEntity.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CEntityManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CSystemManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CWorld.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CBaseComponent.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CTransform.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CTransformSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CSpriteRendererSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CBaseShader.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CBaseShaderLoader.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CQuadSprite.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CBaseTexture2D.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CBaseCamera.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CPerspectiveCamera.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/COrthoCamera.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CBaseShaderCompiler.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CVertexDeclaration.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CBaseGraphicsObjectManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CBaseRenderTarget.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CForwardRenderer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CRenderQueue.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CGraphicsLayersInfo.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CBaseMaterial.cpp"
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
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/CBaseFile.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/CBinaryFileReader.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/CBinaryFileWriter.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/utils/CFileLogger.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/utils/Types.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/utils/CResult.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/utils/Utils.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/math/TVector2.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/math/TVector3.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/math/TVector4.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/math/TMatrix3.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/math/TMatrix4.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/math/TQuaternion.cpp"
	)

source_group("includes" FILES ${TDENGINE2_HEADERS})
source_group("sources" FILES ${TDENGINE2_SOURCES})


if (MSVC) 	#cl.exe compiler's options
	#Debug compiler's options
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /std:c++17 /MDd /W3 /GS /Zc:inline /Od /Gm /Zi  /Zc:wchar_t")
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
		message(STATUS "C++14 is enabled")

		set(CMAKE_CXX_STANDARD 14)
		set(CMAKE_CXX_STANDARD_REQUIRED ON)
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

	target_link_libraries("${TDENGINE2_LIBRARY_NAME}" PUBLIC pthread)
endif ()

#set up stb library headers
target_include_directories("${TDENGINE2_LIBRARY_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/deps/stb")

#build plugins
add_subdirectory(plugins)

if (UNIX)
	install (TARGETS ${TDENGINE2_LIBRARY_NAME} DESTINATION "/usr/lib/TDEngine2")
endif ()

# copy the engine's dll into /tests subdirectory for its testing
#add_custom_command(TARGET ${TDENGINE2_LIBRARY_NAME} POST_BUILD
#	COMMAND ${CMAKE_COMMAND} -E copy
#	"$<TARGET_FILE:${TDENGINE2_LIBRARY_NAME}>"
#	"${CMAKE_CURRENT_SOURCE_DIR}/../tests/bin/")