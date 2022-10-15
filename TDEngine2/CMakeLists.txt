cmake_minimum_required (VERSION 3.8)

project (TDEngine2 
	VERSION 0.6.12
	LANGUAGES CXX)

# The options which are related with plugins are placed here
option(IS_PLUGIN_BUILDING_ENABLED "The option shows whether plugins should be built or not" ON)
option(BUILD_D3D11_GCTX_PLUGIN "Build D3D11 Graphics context plugin" ON)
option(BUILD_WINDOWS_INPUTCTX_PLUGIN "Build Windows input context plugin" ON)
option(BUILD_GL3_0_GCTX_PLUGIN "Build GL3.0 Graphics context plugin" ON)
option(BUILD_UNIX_INPUTCTX_PLUGIN "Build UNIX input context plugin" ON)
option(BUILD_LEVEL_STREAMING_PLUGIN "Build Level streaming utils plugin" ON)
option(IS_TOOLS_BUILDING_ENABLED "The option shows whether engine's toolset should be built or not" ON)
option(USE_EXTERNAL_BULLET_LIBRARY "The options determines whether bullet from deps/ directory or system one will be used" OFF)
option(USE_EXTERNAL_ZLIB_LIBRARY "The options determines whether zlib from deps/ directory or system one will be used" OFF)
option(OPTICK_PROFILER_ENABLED "The option enables/disables usage of optick as a main profiler" ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/$<CONFIGURATION>/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/$<CONFIGURATION>/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/$<CONFIGURATION>/")

if (NOT DEFINED ${TDENGINE2_LIBRARY_NAME})
	set(TDENGINE2_LIBRARY_NAME "TDEngine2")
endif ()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/deps/cmake-modules/")


# 
# Code generation stage
#

set(TDE2_INTROSPECTOR_UTILITY "${CMAKE_CURRENT_SOURCE_DIR}/tools/Introspector/tde2_introspector")

set(TDENGINE2_CGEN_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include/")
set(TDENGINE2_CGEN_INPUT_PATH ${TDENGINE2_CGEN_OUTPUT_PATH})
set(TDENGINE2_CGEN_HEADER "${TDENGINE2_CGEN_OUTPUT_PATH}metadata.h")

message(STATUS "Code generation output: " ${TDENGINE2_CGEN_HEADER})
#file(WRITE ${TDENGINE2_CGEN_HEADER})

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/Config.h.in"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/Config.h")

set(TDENGINE2_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/TDEngine2.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CBaseObject.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CBaseEngineCoreBuilder.h"
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
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/Event.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IEventManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CEventManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IInputContext.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IInputDevice.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CConfigFileEngineCoreBuilder.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IFont.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CFont.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CRuntimeFont.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IImGUIContext.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/Serialization.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/localization/ILocalizationManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/localization/CLocalizationManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/localization/ILocalizationPackage.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/localization/CLocalizationPackage.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IAudioContext.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/Meta.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CGameUserSettings.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CProjectSettings.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CProxyGraphicsContext.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IResourcesRuntimeManifest.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CResourcesRuntimeManifest.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/EntryPoint.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/IGameModesManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CGameModesManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/CProxyInputContext.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/game/ISaveManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/game/CSaveManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/game/CSaveData.h"
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
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CSkinnedMeshRendererSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CBaseSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CPhysics3DSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CObjectsSelectionSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CBoundsUpdatingSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CLightingSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CAnimationSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/components/CBoundsComponent.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CParticlesSimulationSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CUIElementsProcessSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CUIElementsRenderSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CUIEventsSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CMeshAnimatorUpdatingSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/CLODMeshSwitchSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/scene/components/ShadowMappingComponents.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/scene/components/ILight.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/scene/components/CBaseLight.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/scene/components/CDirectionalLight.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/scene/components/CPointLight.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/scene/components/AudioComponents.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/scene/components/CLODStrategyComponent.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/scene/ISceneManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/scene/CSceneManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/scene/IScene.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/scene/CScene.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/scene/IPrefabsRegistry.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/scene/IPrefabsManifest.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/scene/CPrefabsRegistry.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/scene/CPrefabsManifest.h"
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
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CSkinnedMesh.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CStaticMesh.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CBaseMesh.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/VertexData.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IStaticMeshContainer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CStaticMeshContainer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/ISkinnedMeshContainer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CSkinnedMeshContainer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IFramePostProcessor.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CFramePostProcessor.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IPostProcessingProfile.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CBasePostProcessingProfile.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IGeometryBuilder.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CGeometryBuilder.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IAtlasSubTexture.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CAtlasSubTexture.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/ISkeleton.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CSkeleton.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IStructuredBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/animation/IAnimationClip.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/animation/CAnimationClip.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/animation/IAnimationTrack.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/animation/CBaseAnimationTrack.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/animation/AnimationTracks.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/animation/CAnimationContainerComponent.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/animation/CAnimationCurve.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/animation/CMeshAnimatorComponent.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/effects/IParticleEffect.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/effects/CParticleEffect.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/effects/CParticleEmitterComponent.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/effects/ParticleEmitters.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/effects/TParticle.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/UI/CLayoutElementComponent.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/UI/CCanvasComponent.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/UI/CUIElementMeshDataComponent.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/UI/CImageComponent.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/UI/CInputReceiverComponent.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/UI/CLabelComponent.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/UI/C9SliceImageComponent.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/UI/GroupLayoutComponents.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/audio/IAudioSource.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/autotests/CTestContext.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/autotests/ITestFixture.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/autotests/ITestCase.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/autotests/CBaseTestCase.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/autotests/CBaseTestFixture.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/autotests/ITestResultsReporter.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/autotests/CTestResultsTextReporter.h"
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
	"${CMAKE_CURRENT_SOURCE_DIR}/include/editor/CRenderTargetViewerWindow.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/editor/CSceneHierarchyWindow.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/editor/Inspectors.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/editor/CMemoryProfiler.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/editor/CAnimationCurveEditorWindow.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/editor/CProjectSettingsWindow.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/editor/EditorUtils.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/editor/CResourcesBuildManifest.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/editor/CStatsViewerWindow.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/win32/CWin32WindowSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/win32/CWin32Timer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/win32/CWin32DLLManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/win32/CWin32FileSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CTextFileReader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CTextFileWriter.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CCsvFileReader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CConfigFileReader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CBaseFile.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CBinaryFileReader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CBinaryFileWriter.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CImageFileWriter.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CYAMLFile.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CBinaryMeshFileReader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/IOStreams.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/MountableStorages.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CPackageFile.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/BinaryArchives.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/unix/CUnixWindowSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/unix/CUnixDLLManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/unix/CUnixFileSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/unix/CUnixTimer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/platform/CProxyWindowSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/CFileLogger.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/Color.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/Config.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/ILogger.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/ITimer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/Types.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/Utils.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/CU8String.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/CResourceContainer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/CContainers.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/CGradientColor.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/utils/CProgramOptions.h"
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
	"${CMAKE_CURRENT_SOURCE_DIR}/include/physics/3D/CConvexHullCollisionObject3D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/physics/3D/ITrigger3D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/physics/3D/CTrigger3D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/physics/ICollisionObject.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/physics/IRaycastContext.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/physics/CBaseRaycastContext.h"
	)

set(TDENGINE2_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CBaseObject.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CBaseEngineCoreBuilder.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CDefaultEngineCoreBuilder.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CEngineCore.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CBaseFileSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CResourceManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CBaseJobManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CBasePluginManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CBaseResource.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/memory/CBaseAllocator.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/memory/CLinearAllocator.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/memory/CStackAllocator.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/memory/CPoolAllocator.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CEventManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CConfigFileEngineCoreBuilder.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CFont.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CRuntimeFont.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/localization/CLocalizationManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/localization/CLocalizationPackage.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CGameUserSettings.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CProjectSettings.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CProxyGraphicsContext.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/Meta.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CResourcesRuntimeManifest.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/Serialization.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CGameModesManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/core/CProxyInputContext.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/autotests/CTestContext.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/autotests/CBaseTestCase.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/autotests/CBaseTestFixture.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/autotests/CTestResultsTextReporter.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/game/CSaveManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/game/CSaveData.cpp"
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
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CSkinnedMeshRendererSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CPhysics3DSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CObjectsSelectionSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CBoundsUpdatingSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CLightingSystem.cpp"	
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CAnimationSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/component/CBoundsComponent.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CParticlesSimulationSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CUIElementsProcessSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CUIElementsRenderSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CUIEventsSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CMeshAnimatorUpdatingSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CBaseSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ecs/CLODMeshSwitchSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/scene/components/ShadowMappingComponents.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/scene/components/CBaseLight.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/scene/components/CDirectionalLight.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/scene/components/CPointLight.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/scene/components/AudioComponents.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/scene/components/CLODStrategyComponent.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/scene/CSceneManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/scene/CScene.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/scene/CPrefabsManifest.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/scene/CPrefabsRegistry.cpp"
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
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CBaseMesh.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CSkinnedMesh.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CStaticMesh.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CStaticMeshContainer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CSkinnedMeshContainer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CFramePostProcessor.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CBasePostProcessingProfile.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CGeometryBuilder.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CAtlasSubTexture.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/CSkeleton.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/animation/CAnimationClip.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/animation/AnimationTracks.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/animation/CAnimationContainerComponent.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/animation/CAnimationCurve.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/animation/CMeshAnimatorComponent.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/effects/CParticleEffect.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/effects/CParticleEmitterComponent.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/effects/ParticleEmitters.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/UI/CLayoutElementComponent.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/UI/CCanvasComponent.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/UI/CUIElementMeshDataComponent.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/UI/CImageComponent.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/UI/CInputReceiverComponent.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/UI/CLabelComponent.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/UI/C9SliceImageComponent.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/UI/GroupLayoutComponents.cpp"
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
	"${CMAKE_CURRENT_SOURCE_DIR}/source/editor/CRenderTargetViewerWindow.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/editor/CSceneHierarchyWindow.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/editor/Inspectors.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/editor/CMemoryProfiler.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/editor/CAnimationCurveEditorWindow.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/editor/CProjectSettingsWindow.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/editor/EditorUtils.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/editor/CResourcesBuildManifest.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/editor/CStatsViewerWindow.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/win32/CWin32WindowSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/win32/CWin32Timer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/win32/CWin32DLLManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/win32/CWin32FileSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/unix/CUnixWindowSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/unix/CUnixDLLManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/unix/CUnixTimer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/unix/CUnixFileSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/CTextFileReader.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/CTextFileWriter.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/CCsvFileReader.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/CConfigFileReader.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/CBaseFile.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/CBinaryFileReader.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/CBinaryFileWriter.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/CImageFileWriter.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/CYAMLFile.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/CBinaryMeshFileReader.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/IOStreams.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/MountableStorages.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/CPackageFile.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/BinaryArchives.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/platform/CProxyWindowSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/utils/CFileLogger.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/utils/Types.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/utils/Utils.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/utils/CU8String.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/utils/CGradientColor.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/utils/CProgramOptions.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/utils/ScopedPtrDefinitions.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/deps/argparse/argparse.c"
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
	"${CMAKE_CURRENT_SOURCE_DIR}/source/physics/3D/CConvexHullCollisionObject3D.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/physics/3D/CTrigger3D.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/physics/CBaseRaycastContext.cpp"
	)

source_group("includes" FILES ${TDENGINE2_HEADERS})
source_group("sources" FILES ${TDENGINE2_SOURCES})


if (MSVC) 	#cl.exe compiler's options
	message(STATUS "Windows system has detected...")
	
	add_definitions(-D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
	
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
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /WX /MDd /W3 /GS /Zc:inline /Od /Zi  /Zc:wchar_t")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /RTC1 /Gd /Oy- /EHsc /nologo /diagnostics:classic /errorReport:prompt /sdl /permissive- /analyze- /wd4250")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D _DEBUG")

	#Release compiler's options
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /permissive- /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /O2 /sdl /Zc:inline /wd4250")
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

	
	if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -v")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
    endif()
endif ()

#build dependencies
add_subdirectory(deps)


add_library(${TDENGINE2_LIBRARY_NAME} SHARED ${TDENGINE2_SOURCES} ${TDENGINE2_HEADERS} ${TDENGINE2_CGEN_HEADER})


if (OPTICK_PROFILER_ENABLED AND MSVC)
	add_definitions(-DOPTICK_EXPORT)
	add_subdirectory(deps/optick)	
	
	target_include_directories(${TDENGINE2_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/deps/optick/include")
	target_link_libraries(${TDENGINE2_LIBRARY_NAME} PUBLIC OptickCore)
endif ()


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

#set up Wrench library headers
target_include_directories(${TDENGINE2_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/deps/Wrench/source")

target_include_directories(${TDENGINE2_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/deps/debugbreak")

# set up backtrace-cpp library
target_link_libraries(${TDENGINE2_LIBRARY_NAME} PUBLIC ${BACKWARD_ENABLED})
add_backward(${TDENGINE2_LIBRARY_NAME})

# set up argparse library
target_include_directories(${TDENGINE2_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/deps/argparse")


# link Box2D library
target_include_directories(${TDENGINE2_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/deps/Box2D/")
add_dependencies(${TDENGINE2_LIBRARY_NAME} "Box2D")
target_link_libraries(${TDENGINE2_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/deps/Box2D/bin/$<CONFIGURATION>/Box2D${CMAKE_STATIC_LIBRARY_SUFFIX}" )

#link Bullet3 library
if (USE_EXTERNAL_BULLET_LIBRARY)	
	message(STATUS "Bullet3 library's found")

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

#set up rapidcsv header
target_include_directories(${TDENGINE2_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/deps/rapidcsv")

target_include_directories(${TDENGINE2_LIBRARY_NAME} PUBLIC ${BULLET_INCLUDE_DIR})
target_link_libraries(${TDENGINE2_LIBRARY_NAME} PUBLIC ${BULLET_LIBRARIES})

# link zlib
if (USE_EXTERNAL_ZLIB_LIBRARY)	
    find_package(ZLIB REQUIRED)
else ()
	# use zlib from deps/ directory
	message(STATUS "Builtin zlib library is being used...")

	set (ZLIB_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib/")
	set (ZLIB_LIBRARIES 
		#"${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib/$<CONFIGURATION>/${CMAKE_STATIC_LIBRARY_PREFIX}zlib${CMAKE_STATIC_LIBRARY_SUFFIX}"
		"${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib/$<CONFIGURATION>/${CMAKE_STATIC_LIBRARY_PREFIX}zlibstatic${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif ()

target_include_directories(${TDENGINE2_LIBRARY_NAME} PUBLIC ${ZLIB_INCLUDE_DIRS})
target_link_libraries(${TDENGINE2_LIBRARY_NAME} PUBLIC ${ZLIB_LIBRARIES})

#build plugins
if (IS_PLUGIN_BUILDING_ENABLED)
	add_subdirectory(plugins)
endif ()

#build engine's toolset
if (IS_TOOLS_BUILDING_ENABLED)
	add_subdirectory(tools)
endif ()

if (UNIX)
	target_link_libraries(${TDENGINE2_LIBRARY_NAME} PUBLIC pthread stdc++fs)

	install (TARGETS ${TDENGINE2_LIBRARY_NAME} DESTINATION "/usr/lib/TDEngine2")
endif ()

# copy the engine's dll into /tests subdirectory for its testing
add_custom_command(TARGET ${TDENGINE2_LIBRARY_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_SOURCE_DIR}/../${TDENGINE2_TESTS_NAME}/bin/$<CONFIGURATION>/"
	COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${TDENGINE2_LIBRARY_NAME}>"	"${PROJECT_SOURCE_DIR}/../${TDENGINE2_TESTS_NAME}/bin/$<CONFIGURATION>/")


if (UNIX)
	set (HEADER_PATHS_TO_EXCLUDE "$/platform/win32/")
	set (TYPEPATTERNS_TO_EXCLUDE "(.*?)Win32(.*?)")
else ()
	set (HEADER_PATHS_TO_EXCLUDE "/platform/unix/")
	set (TYPEPATTERNS_TO_EXCLUDE "(.*?)Unix(.*?)")
endif ()

#add_custom_command(
#	TARGET ${TDENGINE2_LIBRARY_NAME}
#	PRE_BUILD
#    COMMAND ${TDE2_INTROSPECTOR_UTILITY} ${TDENGINE2_CGEN_INPUT_PATH} -O ${TDENGINE2_CGEN_OUTPUT_PATH} 
#	--emit-enums --emit-classes --emit-structs 
#	--exclude-paths=${HEADER_PATHS_TO_EXCLUDE} 
#	--exclude-typenames=${TYPEPATTERNS_TO_EXCLUDE}
#    COMMENT "Run code generation stage"
#)