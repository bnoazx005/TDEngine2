cmake_minimum_required (VERSION 3.8)

project (TDEngine2 CXX)

# The options which are related with plugins are placed here
option(IS_PLUGIN_BUILDING_ENABLED "The option shows whether plugins should be built or not" ON)
option(BUILD_D3D11_GCTX_PLUGIN "Build D3D11 Graphics context plugin" ON)
option(BUILD_WINDOWS_INPUTCTX_PLUGIN "Build Windows input context plugin" ON)
option(BUILD_GL3_0_GCTX_PLUGIN "Build GL3.0 Graphics context plugin" ON)
option(BUILD_UNIX_INPUTCTX_PLUGIN "Build UNIX input context plugin" ON)
option(IS_TOOLS_BUILDING_ENABLED "The option shows whether engine's toolset should be built or not" ON)
option(USE_EXTERNAL_BULLET_LIBRARY "The options determines whether bullet from deps/ directory or system one will be used" OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/$<CONFIGURATION>/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/$<CONFIGURATION>/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/$<CONFIGURATION>/")

if (NOT DEFINED ${TDENGINE2_LIBRARY_NAME})
	set(TDENGINE2_LIBRARY_NAME "TDEngine2")
endif ()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/deps/cmake-modules/")

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
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CConfigFileEngineCoreBuilder.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IFont.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CFont.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IImGUIContext.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/Serialization.h"
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
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/ICameraSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CCameraSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CPhysics2DSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CStaticMeshRendererSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CBaseSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CPhysics3DSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CObjectsSelectionSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CBoundsUpdatingSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/components/CBoundsComponent.h"
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
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/ICubemapTexture.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CBaseTexture2D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CBaseCubemapTexture.h"
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
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IGlobalShaderProperties.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CGlobalShaderProperties.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/ITextureAtlas.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CTextureAtlas.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IDebugUtility.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CDebugUtility.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IMesh.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CStaticMesh.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/VertexData.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IStaticMeshContainer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CStaticMeshContainer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IFramePostProcessor.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CFramePostProcessor.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IPostProcessingProfile.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CBasePostProcessingProfile.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IGeometryBuilder.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CGeometryBuilder.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/animation/IAnimationClip.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/animation/CAnimationClip.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/animation/IAnimationTrack.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/editor/IProfiler.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/editor/CPerfProfiler.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/editor/IEditorsManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/editor/CEditorsManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/editor/IEditorWindow.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/editor/CProfilerEditorWindow.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/editor/CLevelEditorWindow.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/editor/ecs/CEditorCameraControlSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/editor/ecs/EditorComponents.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/editor/CDevConsoleWindow.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/editor/ISelectionManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/editor/CSelectionManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/editor/EditorActions.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/editor/CEditorActionsManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/win32/CWin32WindowSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/win32/CWin32Timer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/win32/CWin32DLLManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/win32/CWin32FileSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CTextFileReader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CConfigFileReader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CBaseFile.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CBinaryFileReader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CBinaryFileWriter.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CImageFileWriter.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CYAMLFile.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CBinaryMeshFileReader.h"
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
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/CU8String.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/CResult.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/CResourceContainer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/CContainers.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/math/TVector2.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/math/TVector3.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/math/TVector4.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/math/TMatrix3.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/math/TMatrix4.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/math/TQuaternion.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/math/TRect.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/math/TPlane.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/math/TAABB.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/math/MathUtils.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/math/TRay.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/physics/2D/ICollisionObject2D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/physics/2D/CBaseCollisionObject2D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/physics/2D/CBoxCollisionObject2D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/physics/2D/CCircleCollisionObject2D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/physics/2D/ICollisionObjectsVisitor.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/physics/2D/ITrigger2D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/physics/2D/CTrigger2D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/physics/3D/ICollisionObject3D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/physics/3D/CBaseCollisionObject3D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/physics/3D/CBoxCollisionObject3D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/physics/3D/ICollisionObjects3DVisitor.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/physics/3D/CSphereCollisionObject3D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/physics/ICollisionObject.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/physics/IRaycastContext.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/physics/CBaseRaycastContext.h"
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
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CConfigFileEngineCoreBuilder.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CFont.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CComponentManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CEntity.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CEntityManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CSystemManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CWorld.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CBaseComponent.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CTransform.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CTransformSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CSpriteRendererSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CCameraSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CPhysics2DSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CStaticMeshRendererSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CPhysics3DSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CObjectsSelectionSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CBoundsUpdatingSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/component/CBoundsComponent.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CBaseShader.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CBaseShaderLoader.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CQuadSprite.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CBaseTexture2D.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CBaseCubemapTexture.cpp"
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
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CGlobalShaderProperties.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CTextureAtlas.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CDebugUtility.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CStaticMesh.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CStaticMeshContainer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CFramePostProcessor.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CBasePostProcessingProfile.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CGeometryBuilder.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/animation/CAnimationClip.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/editor/CPerfProfiler.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/editor/CEditorsManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/editor/IEditorWindow.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/editor/CProfilerEditorWindow.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/editor/CLevelEditorWindow.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/editor/ecs/CEditorCameraControlSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/editor/ecs/EditorComponents.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/editor/CDevConsoleWindow.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/editor/CSelectionManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/editor/CEditorActionsManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/editor/EditorActions.cpp"
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
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/CImageFileWriter.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/CYAMLFile.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/CBinaryMeshFileReader.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/utils/CFileLogger.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/utils/Types.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/utils/CResult.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/utils/Utils.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/utils/CU8String.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/math/TVector2.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/math/TVector3.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/math/TVector4.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/math/TMatrix3.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/math/TMatrix4.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/math/TQuaternion.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/math/TPlane.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/math/TAABB.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/physics/2D/CBaseCollisionObject2D.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/physics/2D/CBoxCollisionObject2D.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/physics/2D/CCircleCollisionObject2D.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/physics/2D/CTrigger2D.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/physics/3D/CBaseCollisionObject3D.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/physics/3D/CBoxCollisionObject3D.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/physics/3D/CSphereCollisionObject3D.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/physics/CBaseRaycastContext.cpp"
	)

source_group("includes" FILES ${TDENGINE2_HEADERS})
source_group("sources" FILES ${TDENGINE2_SOURCES})


if (MSVC) 	#cl.exe compiler's options
	message(STATUS "Windows system has detected...")
	
	include(CheckCXXCompilerFlag)

	CHECK_CXX_COMPILER_FLAG("/std:c++14" COMPILER_SUPPORTS_CXX14)

	if(COMPILER_SUPPORTS_CXX14)
		message(STATUS "C++14 is enabled")

		set(CMAKE_CXX_STANDARD 14)
		set(CMAKE_CXX_STANDARD_REQUIRED ON)
	else()
		message(ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
	endif()

	#Debug compiler's options
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /W3 /GS /Zc:inline /Od /Zi  /Zc:wchar_t")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /RTC1 /Gd /Oy- /EHsc /nologo /diagnostics:classic /errorReport:prompt /sdl /permissive- /analyze-")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D _DEBUG")

	#Release compiler's options
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /permissive- /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /O2 /sdl /Zc:inline")
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

#build dependencies
add_subdirectory(deps)


add_library(${TDENGINE2_LIBRARY_NAME} SHARED ${TDENGINE2_SOURCES} ${TDENGINE2_HEADERS})

if (UNIX)
	# disable lib prefix
	SET_TARGET_PROPERTIES(${TDENGINE2_LIBRARY_NAME} PROPERTIES PREFIX "")
endif ()

# disable the <libname>_EXPORTS
set_target_properties(${TDENGINE2_LIBRARY_NAME} PROPERTIES DEFINE_SYMBOL "")

#set up GLEW headers
target_include_directories(${TDENGINE2_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/deps/glew-2.1.0/include")

if (UNIX)
	FIND_PACKAGE(X11 REQUIRED)	# include X11 library

	target_include_directories(${TDENGINE2_LIBRARY_NAME} PUBLIC ${X11_INCLUDE_DIR})
	target_link_libraries(${TDENGINE2_LIBRARY_NAME} PUBLIC ${X11_LIBRARIES})

	target_link_libraries(${TDENGINE2_LIBRARY_NAME} PUBLIC pthread)
endif ()

#set up stb library headers
target_include_directories(${TDENGINE2_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/deps/stb")

# link Box2D library
target_include_directories(${TDENGINE2_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/deps/Box2D/")
add_dependencies(${TDENGINE2_LIBRARY_NAME} "Box2D")
target_link_libraries(${TDENGINE2_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/deps/Box2D/bin/$<CONFIGURATION>/Box2D${CMAKE_STATIC_LIBRARY_SUFFIX}" )

#link Bullet3 library
if (USE_EXTERNAL_BULLET_LIBRARY)	
    find_package(Bullet REQUIRED)
else ()
	# use Bullet3 from deps/ directory
	message(STATUS "Builtin bullet3 library is being used...")

	set (BULLET_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/bullet/src/")
	set (BULLET_LIBRARIES 
		"${CMAKE_CURRENT_SOURCE_DIR}/deps/bullet3/lib/$<CONFIGURATION>/${CMAKE_STATIC_LIBRARY_PREFIX}BulletDynamics${CMAKE_STATIC_LIBRARY_SUFFIX}"
		"${CMAKE_CURRENT_SOURCE_DIR}/deps/bullet3/lib/$<CONFIGURATION>/${CMAKE_STATIC_LIBRARY_PREFIX}BulletCollision${CMAKE_STATIC_LIBRARY_SUFFIX}"
		"${CMAKE_CURRENT_SOURCE_DIR}/deps/bullet3/lib/$<CONFIGURATION>/${CMAKE_STATIC_LIBRARY_PREFIX}LinearMath${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif ()

target_include_directories(${TDENGINE2_LIBRARY_NAME} PUBLIC ${BULLET_INCLUDE_DIR})
target_link_libraries(${TDENGINE2_LIBRARY_NAME} PUBLIC ${BULLET_LIBRARIES})


#build plugins
if (IS_PLUGIN_BUILDING_ENABLED)
	add_subdirectory(plugins)
endif ()

#build engine's toolset
if (IS_TOOLS_BUILDING_ENABLED)
	add_subdirectory(tools)
endif ()

if (UNIX)
	install (TARGETS ${TDENGINE2_LIBRARY_NAME} DESTINATION "/usr/lib/TDEngine2")
endif ()

# copy the engine's dll into /tests subdirectory for its testing
add_custom_command(TARGET ${TDENGINE2_LIBRARY_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_SOURCE_DIR}/../${TDENGINE2_TESTS_NAME}/bin/$<CONFIGURATION>/"
	COMMAND ${CMAKE_COMMAND} -E copy
	"$<TARGET_FILE:${TDENGINE2_LIBRARY_NAME}>"
	"${PROJECT_SOURCE_DIR}/../${TDENGINE2_TESTS_NAME}/bin/$<CONFIGURATION>/")