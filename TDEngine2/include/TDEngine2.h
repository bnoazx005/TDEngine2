/*!
	/file TDEngine.h
	/date 14.09.2018
	/authors Kasimov Ildar
*/

#pragma once

///core
#include "core/IEngineCore.h"
#include "core/IEngineSubsystem.h"
#include "core/IWindowSystem.h"
#include "core/IGraphicsContext.h"
#include "core/CEngineCore.h"
#include "core/IEngineCoreBuilder.h"
#include "core/CBaseEngineCoreBuilder.h"
#include "core/CDefaultEngineCoreBuilder.h"
#include "core/IEngineListener.h"
#include "core/IBaseObject.h"
#include "core/CBaseObject.h"
#include "core/IPlugin.h"
#include "core/IDLLManager.h"
#include "core/IFileSystem.h"
#include "core/IFile.h"
#include "core/CBaseFileSystem.h"
#include "core/IResource.h"
#include "core/CBaseResource.h"
#include "core/IResourceManager.h"
#include "core/IResourceLoader.h"
#include "core/CResourceManager.h"
#include "core/IJobManager.h"
#include "core/CBaseJobManager.h"
#include "core/IPluginManager.h"
#include "core/CBasePluginManager.h"
#include "core/IResourceFactory.h"
#include "core/memory/IAllocator.h"
#include "core/memory/CBaseAllocator.h"
#include "core/memory/CLinearAllocator.h"
#include "core/memory/CStackAllocator.h"
#include "core/memory/CPoolAllocator.h"
#include "core/Event.h"
#include "core/IEventManager.h"
#include "core/CEventManager.h"
#include "core/IInputContext.h"
#include "core/IInputDevice.h"
#include "core/CConfigFileEngineCoreBuilder.h"
#include "core/IFont.h"
#include "core/CFont.h"
#include "core/CRuntimeFont.h"
#include "core/IImGUIContext.h"
#include "core/Serialization.h"
#include "core/localization/ILocalizationManager.h"
#include "core/localization/ILocalizationPackage.h"
#include "core/localization/CLocalizationManager.h"
#include "core/localization/CLocalizationPackage.h"
#include "core/IAudioContext.h"
#include "core/Meta.h"
#include "core/CGameUserSettings.h"
#include "core/CProjectSettings.h"
#include "core/CProxyGraphicsContext.h"
#include "core/IResourcesRuntimeManifest.h"
#include "core/CResourcesRuntimeManifest.h"
#include "core/EntryPoint.h"
#include "core/IGameModesManager.h"
#include "core/CGameModesManager.h"
#include "core/CProxyInputContext.h"

/// game
#include "game/ISaveManager.h"
#include "game/CSaveManager.h"
#include "game/CSaveData.h"

///ecs
#include "ecs/CEntity.h"
#include "ecs/CEntityManager.h"
#include "ecs/CComponentManager.h"
#include "ecs/IComponent.h"
#include "ecs/CBaseComponent.h"
#include "ecs/ISystem.h"
#include "ecs/ISystemManager.h"
#include "ecs/CSystemManager.h"
#include "ecs/IWorld.h"
#include "ecs/CWorld.h"
#include "ecs/ITransform.h"
#include "ecs/CTransform.h"
#include "ecs/IComponentFactory.h"
#include "ecs/IComponentManager.h"
#include "ecs/CTransformSystem.h"
#include "ecs/CSpriteRendererSystem.h"
#include "ecs/ICameraSystem.h"
#include "ecs/CCameraSystem.h"
#include "ecs/CPhysics2DSystem.h"
#include "ecs/CStaticMeshRendererSystem.h"
#include "ecs/CSkinnedMeshRendererSystem.h"
#include "ecs/CBaseSystem.h"
#include "ecs/CObjectsSelectionSystem.h"
#include "ecs/CBoundsUpdatingSystem.h"
#include "ecs/CLightingSystem.h"
#include "ecs/CAnimationSystem.h"
#include "ecs/CParticlesSimulationSystem.h"
#include "ecs/CUIElementsProcessSystem.h"
#include "ecs/CUIElementsRenderSystem.h"
#include "ecs/CUIEventsSystem.h"
#include "ecs/CMeshAnimatorUpdatingSystem.h"
#include "ecs/components/CBoundsComponent.h"
#include "ecs/CLODMeshSwitchSystem.h"
#include "ecs/CWeatherSystem.h"
#include "ecs/CSplashScreenLogicSystem.h"

///graphics
#include "graphics/IBuffer.h"
#include "graphics/IVertexDeclaration.h"
#include "graphics/IShader.h"
#include "graphics/CBaseShader.h"
#include "graphics/IShaderCompiler.h"
#include "graphics/CBaseShaderLoader.h"
#include "graphics/ISprite.h"
#include "graphics/CQuadSprite.h"
#include "graphics/ITexture.h"
#include "graphics/ITexture2D.h"
#include "graphics/ITexture3D.h"
#include "graphics/ICubemapTexture.h"
#include "graphics/CBaseTexture2D.h"
#include "graphics/CBaseCubemapTexture.h"
#include "graphics/ICamera.h"
#include "graphics/CBaseCamera.h"
#include "graphics/CPerspectiveCamera.h"
#include "graphics/COrthoCamera.h"
#include "graphics/CBaseShaderCompiler.h"
#include "graphics/CVertexDeclaration.h"
#include "graphics/InternalShaderData.h"
#include "graphics/IGraphicsObjectManager.h"
#include "graphics/CBaseGraphicsObjectManager.h"
#include "graphics/IRenderable.h"
#include "graphics/IRenderer.h"
#include "graphics/CForwardRenderer.h"
#include "graphics/CRenderQueue.h"
#include "graphics/IGraphicsLayersInfo.h"
#include "graphics/CGraphicsLayersInfo.h"
#include "graphics/IMaterial.h"
#include "graphics/CBaseMaterial.h"
#include "graphics/IGlobalShaderProperties.h"
#include "graphics/CGlobalShaderProperties.h"
#include "graphics/ITextureAtlas.h"
#include "graphics/CTextureAtlas.h"
#include "graphics/IDebugUtility.h"
#include "graphics/CDebugUtility.h"
#include "graphics/IMesh.h"
#include "graphics/CBaseMesh.h"
#include "graphics/CStaticMesh.h"
#include "graphics/IStaticMeshContainer.h"
#include "graphics/CStaticMeshContainer.h"
#include "graphics/ISkinnedMeshContainer.h"
#include "graphics/CSkinnedMeshContainer.h"
#include "graphics/IPostProcessingProfile.h"
#include "graphics/CBasePostProcessingProfile.h"
#include "graphics/IGeometryBuilder.h"
#include "graphics/CGeometryBuilder.h"
#include "graphics/ISkeleton.h"
#include "graphics/CSkeleton.h"
#include "graphics/animation/IAnimationClip.h"
#include "graphics/animation/CAnimationClip.h"
#include "graphics/animation/IAnimationTrack.h"
#include "graphics/animation/CBaseAnimationTrack.h"
#include "graphics/animation/AnimationTracks.h"
#include "graphics/animation/CAnimationContainerComponent.h"
#include "graphics/animation/CAnimationCurve.h"
#include "graphics/animation/CMeshAnimatorComponent.h"
#include "graphics/effects/IParticleEffect.h"
#include "graphics/effects/CParticleEffect.h"
#include "graphics/effects/CParticleEmitterComponent.h"
#include "graphics/effects/ParticleEmitters.h"
#include "graphics/effects/TParticle.h"
#include "graphics/IAtlasSubTexture.h"
#include "graphics/CAtlasSubTexture.h"
#include "graphics/CBaseTexture3D.h"
#include "graphics/UI/CLayoutElementComponent.h"
#include "graphics/UI/CCanvasComponent.h"
#include "graphics/UI/CUIElementMeshDataComponent.h"
#include "graphics/UI/CImageComponent.h"
#include "graphics/UI/CInputReceiverComponent.h"
#include "graphics/UI/CLabelComponent.h"
#include "graphics/UI/C9SliceImageComponent.h"
#include "graphics/UI/GroupLayoutComponents.h"
#include "graphics/UI/CToggleComponent.h"
#include "graphics/UI/CUISliderComponent.h"
#include "graphics/UI/CInputFieldComponent.h"
#include "graphics/UI/CScrollableUIAreaComponent.h"
#include "graphics/UI/CDropDownComponent.h"
#include "graphics/CFrameGraph.h"
#include "graphics/CFrameGraphResources.h"
#include "graphics/IGraphicsPipeline.h"
#include "graphics/CBaseGraphicsPipeline.h"

/// audio
#include "audio/IAudioSource.h"

/// autotests
#include "autotests/CTestContext.h"
#include "autotests/ITestFixture.h"
#include "autotests/ITestCase.h"
#include "autotests/CBaseTestCase.h"
#include "autotests/CBaseTestFixture.h"
#include "autotests/ITestResultsReporter.h"
#include "autotests/CTestResultsTextReporter.h"

///editor
#include "editor/IProfiler.h"
#include "editor/CPerfProfiler.h"
#include "editor/IEditorsManager.h"
#include "editor/CEditorsManager.h"
#include "editor/IEditorWindow.h"
#include "editor/CProfilerEditorWindow.h"
#include "editor/CLevelEditorWindow.h"
#include "editor/ecs/CEditorCameraControlSystem.h"
#include "editor/ecs/EditorComponents.h"
#include "editor/CDevConsoleWindow.h"
#include "editor/ISelectionManager.h"
#include "editor/CSelectionManager.h"
#include "editor/EditorActions.h"
#include "editor/CEditorActionsManager.h"
#include "editor/CRenderTargetViewerWindow.h"
#include "editor/CSceneHierarchyWindow.h"
#include "editor/Inspectors.h"
#include "editor/CMemoryProfiler.h"
#include "editor/CAnimationCurveEditorWindow.h"
#include "editor/CProjectSettingsWindow.h"
#include "editor/EditorUtils.h"
#include "editor/CResourcesBuildManifest.h"
#include "editor/CStatsViewerWindow.h"
#include "editor/CEditorSettings.h"
#include "editor/CStatsCounters.h"

///math
#include "math/TVector2.h"
#include "math/TVector3.h"
#include "math/TVector4.h"
#include "math/TMatrix3.h"
#include "math/TMatrix4.h"
#include "math/TQuaternion.h"
#include "math/TRect.h"
#include "math/TPlane.h"
#include "math/TAABB.h"
#include "math/MathUtils.h"
#include "math/TRay.h"

///physics
#include "physics/2D/ICollisionObject2D.h"
#include "physics/2D/CBaseCollisionObject2D.h"
#include "physics/2D/CBoxCollisionObject2D.h"
#include "physics/2D/CCircleCollisionObject2D.h"
#include "physics/2D/ICollisionObjectsVisitor.h"
#include "physics/2D/ITrigger2D.h"
#include "physics/2D/CTrigger2D.h"
#include "physics/3D/IBoxCollisionObject3D.h"
#include "physics/3D/ISphereCollisionObject3D.h"
#include "physics/3D/IConvexHullCollisionObject3D.h"
#include "physics/3D/ICapsuleCollisionObject3D.h"
#include "physics/3D/ITrigger3D.h"
#include "physics/ICollisionObject.h"
#include "physics/IRaycastContext.h"
#include "physics/CBaseRaycastContext.h"

/// scene
#include "scene/components/ShadowMappingComponents.h"
#include "scene/components/ILight.h"
#include "scene/components/CBaseLight.h"
#include "scene/components/CDirectionalLight.h"
#include "scene/components/CPointLight.h"
#include "scene/components/CSpotLight.h"
#include "scene/components/AudioComponents.h"
#include "scene/components/CLODStrategyComponent.h"
#include "scene/components/CObjIdComponent.h"
#include "scene/components/CPrefabLinkInfoComponent.h"
#include "scene/components/CWeatherComponent.h"
#include "scene/components/CSplashScreenItemComponent.h"
#include "scene/ISceneManager.h"
#include "scene/CSceneManager.h"
#include "scene/IScene.h"
#include "scene/CScene.h"
#include "scene/IPrefabsRegistry.h"
#include "scene/IPrefabsManifest.h"
#include "scene/CPrefabsRegistry.h"
#include "scene/CPrefabsManifest.h"
#include "scene/CPrefabChangesList.h"

///platform
#include "platform/win32/CWin32WindowSystem.h"
#include "platform/win32/CWin32Timer.h"
#include "platform/win32/CWin32DLLManager.h"
#include "platform/win32/CWin32FileSystem.h"
#include "platform/CTextFileReader.h"
#include "platform/CTextFileWriter.h"
#include "platform/CCsvFileReader.h"
#include "platform/unix/CUnixWindowSystem.h"
#include "platform/unix/CUnixDLLManager.h"
#include "platform/unix/CUnixFileSystem.h"
#include "platform/unix/CUnixTimer.h"
#include "platform/CConfigFileReader.h"
#include "platform/CConfigFileWriter.h"
#include "platform/CBaseFile.h"
#include "platform/CBinaryFileReader.h"
#include "platform/CBinaryFileWriter.h"
#include "platform/CImageFileWriter.h"
#include "platform/CYAMLFile.h"
#include "platform/CBinaryMeshFileReader.h"
#include "platform/IOStreams.h"
#include "platform/MountableStorages.h"
#include "platform/CPackageFile.h"
#include "platform/BinaryArchives.h"
#include "platform/CProxyWindowSystem.h"

///utils
#include "utils/Types.h"
#include "utils/Config.h"
#include "utils/Color.h"
#include "utils/ILogger.h"
#include "utils/CFileLogger.h"
#include "utils/ITimer.h"
#include "utils/Utils.h"
#include "utils/CResourceContainer.h"
#include "utils/CU8String.h"
#include "utils/CContainers.h"
#include "utils/CGradientColor.h"
#include "utils/CProgramOptions.h"

/// D3D11GraphicsContext plugin's header
#if defined (TDE2_BUILD_D3D11_GCTX_PLUGIN)
	#include "./../plugins/D3D11GraphicsContext/include/CD3D11GCtxPlugin.h"
#endif

/// OGLGraphicsContext plugin's header
#if defined (TDE2_BUILD_OGL_GCTX_PLUGIN)
	#include "./../plugins/OGLGraphicsContext/include/COGLGCtxPlugin.h"
#endif

#if defined(TDE2_BUILD_FMOD_CTX_PLUGIN)
	#include "../plugins/FmodAudioContext/include/CFmodAudioCtxPlugin.h"
#endif

#if defined (TDE2_BUILD_BULLET_PHYSICS_PLUGIN)
	#include "../plugins/TDE2BulletPhysics/include/CBulletPhysicsPlugin.h"
#endif