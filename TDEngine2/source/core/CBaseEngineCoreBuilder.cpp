#include "../../include/core/CBaseEngineCoreBuilder.h"
#include "../../include/core/CEngineCore.h"
#include "../../include/core/IPlugin.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/CResourceManager.h"
#include "../../include/core/CBaseJobManager.h"
#include "../../include/core/IJobManager.h"
#include "../../include/core/CBasePluginManager.h"
#include "../../include/core/CEventManager.h"
#include "../../include/core/IInputContext.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/CFont.h"
#include "../../include/core/CRuntimeFont.h"
#include "../../include/core/memory/CLinearAllocator.h"
#include "../../include/core/memory/CPoolAllocator.h"
#include "../../include/core/memory/CStackAllocator.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/core/localization/CLocalizationManager.h"
#include "../../include/core/localization/CLocalizationPackage.h"
#include "../../include/core/CGameUserSettings.h"
#include "../../include/core/CProjectSettings.h"
#include "../../include/core/CProxyGraphicsContext.h"
#include "../../include/core/CResourcesRuntimeManifest.h"
#include "../../include/core/CGameModesManager.h"
#include "../../include/game/CSaveManager.h"
#include "../../include/platform/win32/CWin32WindowSystem.h"
#include "../../include/platform/win32/CWin32FileSystem.h"
#include "../../include/platform/unix/CUnixWindowSystem.h"
#include "../../include/platform/unix/CUnixFileSystem.h"
#include "../../include/platform/CProxyWindowSystem.h"
#include "../../include/platform/CTextFileReader.h"
#include "../../include/platform/CCsvFileReader.h"
#include "../../include/platform/CConfigFileReader.h"
#include "../../include/platform/CBinaryFileReader.h"
#include "../../include/platform/CBinaryFileWriter.h"
#include "../../include/platform/CImageFileWriter.h"
#include "../../include/platform/CYAMLFile.h"
#include "../../include/platform/CPackageFile.h"
#include "../../include/platform/CBinaryMeshFileReader.h"
#include "../../include/platform/BinaryArchives.h"
#include "../../include/graphics/CForwardRenderer.h"
#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/graphics/CTextureAtlas.h"
#include "../../include/graphics/effects/CParticleEffect.h"
#include "../../include/graphics/ITexture2D.h"
#include "../../include/graphics/ICubemapTexture.h"
#include "../../include/graphics/IMesh.h"
#include "../../include/graphics/CSkeleton.h"
#include "../../include/graphics/CAtlasSubTexture.h"
#include "../../include/graphics/CStaticMesh.h"
#include "../../include/graphics/CSkinnedMesh.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/editor/CEditorsManager.h"
#include "../../include/editor/CProfilerEditorWindow.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/editor/CMemoryProfiler.h"
#include "../../include/editor/CLevelEditorWindow.h"
#include "../../include/editor/CDevConsoleWindow.h"
#include "../../include/editor/CSelectionManager.h"
#include "../../include/editor/CRenderTargetViewerWindow.h"
#include "../../include/editor/CProjectSettingsWindow.h"
#include "../../include/editor/Inspectors.h"
#include "../../include/graphics/CFramePostProcessor.h"
#include "../../include/graphics/CBasePostProcessingProfile.h"
#include "../../include/graphics/IDebugUtility.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/animation/CAnimationClip.h"
#include "../../include/scene/CSceneManager.h"
#include "../../include/scene/CPrefabsManifest.h"
#include "../../include/scene/CPrefabsRegistry.h"
#include "../../include/ecs/CWorld.h"
#include "../../include/ecs/CTransform.h"
#include "deferOperation.hpp"
#include <memory>
#include <cstring>
#include <tuple>


namespace TDEngine2
{
	CBaseEngineCoreBuilder::CBaseEngineCoreBuilder() :
		CBaseObject(), 
		mpEngineCoreInstance(nullptr),
		mpWindowSystemInstance(nullptr),
		mpJobManagerInstance(nullptr),
		mpPluginManagerInstance(nullptr),
		mpGraphicsContextInstance(nullptr),
		mpResourceManagerInstance(nullptr)
	{
	}

	E_RESULT_CODE CBaseEngineCoreBuilder::_configureGraphicsContext(const std::string& rendererPluginPath)
	{
		if (!mIsInitialized || !mpPluginManagerInstance)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		if (rendererPluginPath.empty())
		{
			mpGraphicsContextInstance = CreateProxyGraphicsContext(mpWindowSystemInstance, result);
			if (RC_OK != result)
			{
				return result;
			}

			result = mpEngineCoreInstance->RegisterSubsystem(DynamicPtrCast<IEngineSubsystem>(mpGraphicsContextInstance));
			return result;
		}

#if defined (TDE2_USE_WINPLATFORM)
		result = mpPluginManagerInstance->LoadPlugin(rendererPluginPath);
#elif defined (TDE2_USE_UNIXPLATFORM)
		result = mpPluginManagerInstance->LoadPlugin(rendererPluginPath);
#else
#endif

		mpGraphicsContextInstance = mpEngineCoreInstance->GetSubsystem<IGraphicsContext>();

		return RC_OK;
	}

	E_RESULT_CODE CBaseEngineCoreBuilder::_configureAudioContext(const std::string& audioPluginPath)
	{
		if (!mIsInitialized || !mpPluginManagerInstance)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

#if defined (TDE2_USE_WINPLATFORM)
		result = mpPluginManagerInstance->LoadPlugin(audioPluginPath);
#elif defined (TDE2_USE_UNIXPLATFORM)
		result = mpPluginManagerInstance->LoadPlugin(audioPluginPath);
#else
#endif
		if (RC_OK != result)
		{
			LOG_ERROR("[CBaseEngineCoreBuilder] FmodAudioContext's loading failed. Audio components won't be available at runtime");
			return result;
		}

		return result;
	}

	E_RESULT_CODE CBaseEngineCoreBuilder::_configureWindowSystem(const std::string& name, U32 width, U32 height, U32 flags)
	{
		if (!mIsInitialized || !mpEventManagerInstance)
		{
			return RC_FAIL;
		}

		mpWindowSystemInstance = nullptr;

		E_RESULT_CODE result = RC_OK;

		if (flags & E_PARAMETERS::P_WINDOWLESS_MODE)
		{
			mpWindowSystemInstance = TPtr<IWindowSystem>(CreateProxyWindowSystem(mpEventManagerInstance, name, width, height, flags, result));
		}
		else
		{
#if defined (TDE2_USE_WINPLATFORM)																/// Win32 Platform
			mpWindowSystemInstance = TPtr<IWindowSystem>(CreateWin32WindowSystem(mpEventManagerInstance, name, width, height, flags, result));
#elif defined (TDE2_USE_UNIXPLATFORM)
			mpWindowSystemInstance = TPtr<IWindowSystem>(CreateUnixWindowSystem(mpEventManagerInstance, name, width, height, flags, result));
#else
#endif
		}

		if (result != RC_OK)
		{
			return result;
		}

		return mpEngineCoreInstance->RegisterSubsystem(DynamicPtrCast<IEngineSubsystem>(mpWindowSystemInstance));
	}

	E_RESULT_CODE CBaseEngineCoreBuilder::_configureFileSystem()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

#if defined (TDE2_USE_WINPLATFORM)
		mpFileSystemInstance = TPtr<IFileSystem>(CreateWin32FileSystem(result));
#elif defined (TDE2_USE_UNIXPLATFORM)
		mpFileSystemInstance = TPtr<IFileSystem>(CreateUnixFileSystem(result));
#else
#endif

		/// register known file types factories
		if (((result = mpFileSystemInstance->RegisterFileFactory<ITextFileReader>({ CreateTextFileReader, E_FILE_FACTORY_TYPE::READER })) != RC_OK) ||
			((result = mpFileSystemInstance->RegisterFileFactory<ICsvFileReader>({ CreateCsvFileReader, E_FILE_FACTORY_TYPE::READER })) != RC_OK) ||
			((result = mpFileSystemInstance->RegisterFileFactory<IConfigFileReader>({ CreateConfigFileReader, E_FILE_FACTORY_TYPE::READER })) != RC_OK) ||
			((result = mpFileSystemInstance->RegisterFileFactory<IBinaryFileReader>({ CreateBinaryFileReader, E_FILE_FACTORY_TYPE::READER })) != RC_OK) ||
			((result = mpFileSystemInstance->RegisterFileFactory<IBinaryFileWriter>({ CreateBinaryFileWriter, E_FILE_FACTORY_TYPE::WRITER })) != RC_OK) ||
			((result = mpFileSystemInstance->RegisterFileFactory<IImageFileWriter>({ CreateImageFileWriter, E_FILE_FACTORY_TYPE::WRITER })) != RC_OK) ||
			((result = mpFileSystemInstance->RegisterFileFactory<IYAMLFileReader>({ CreateYAMLFileReader, E_FILE_FACTORY_TYPE::READER })) != RC_OK) ||
			((result = mpFileSystemInstance->RegisterFileFactory<IYAMLFileWriter>({ CreateYAMLFileWriter, E_FILE_FACTORY_TYPE::WRITER })) != RC_OK) ||
			((result = mpFileSystemInstance->RegisterFileFactory<IPackageFileReader>({ CreatePackageFileReader, E_FILE_FACTORY_TYPE::READER })) != RC_OK) ||
			((result = mpFileSystemInstance->RegisterFileFactory<IPackageFileWriter>({ CreatePackageFileWriter, E_FILE_FACTORY_TYPE::WRITER })) != RC_OK) ||
			((result = mpFileSystemInstance->RegisterFileFactory<IBinaryMeshFileReader>({ CreateBinaryMeshFileReader, E_FILE_FACTORY_TYPE::READER })) != RC_OK) ||
			((result = mpFileSystemInstance->RegisterFileFactory<IBinaryArchiveWriter>({ CreateBinaryArchiveWriter, E_FILE_FACTORY_TYPE::WRITER })) != RC_OK) ||
			((result = mpFileSystemInstance->RegisterFileFactory<IBinaryArchiveReader>({ CreateBinaryArchiveReader, E_FILE_FACTORY_TYPE::READER })) != RC_OK))
		{
			return result;
		}

		return mpEngineCoreInstance->RegisterSubsystem(DynamicPtrCast<IEngineSubsystem>(mpFileSystemInstance));
	}


	static TPtr<IResourcesRuntimeManifest> LoadResourcesRuntimeManifest(TPtr<IFileSystem> pFileSystem, const std::string& manifestFilepath)
	{
		E_RESULT_CODE result = RC_OK;

		auto&& pResourcesManifest = TPtr<IResourcesRuntimeManifest>(CreateResourcesRuntimeManifest(result));
		if (RC_OK != result)
		{
			LOG_ERROR("[CBaseEngineCoreBuilder] Coudn't load resources runtime manifest");
			return nullptr;
		}

		auto openManifestFileResult = pFileSystem->Open<IYAMLFileReader>(manifestFilepath);
		if (openManifestFileResult.HasError())
		{
			LOG_ERROR(Wrench::StringUtils::Format("[CBaseEngineCoreBuilder] Coudn't find a manifest at filepath: {0}", manifestFilepath));
			return nullptr;
		}

		if (IYAMLFileReader* pManifestFile = pFileSystem->Get<IYAMLFileReader>(openManifestFileResult.Get()))
		{
			pResourcesManifest->SetBaseResourcesPath(pFileSystem->GetParentPath(manifestFilepath));
			pResourcesManifest->Load(pManifestFile);
			pManifestFile->Close();
		}

		return pResourcesManifest;
	}


	E_RESULT_CODE CBaseEngineCoreBuilder::_configureResourceManager()
	{
		if (!mIsInitialized || !mpJobManagerInstance || !mpFileSystemInstance)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		mpResourceManagerInstance = TPtr<IResourceManager>(CreateResourceManager(
													mpJobManagerInstance, 
													LoadResourcesRuntimeManifest(mpFileSystemInstance, CProjectSettings::Get()->mCommonSettings.mPathToResourcesRuntimeManifest), 
													result));

		if (result != RC_OK)
		{
			return result;
		}

		/// \todo Replace it with reading policies from config file 
#if TDE2_RESOURCES_STREAMING_ENABLED
		mpResourceManagerInstance->RegisterTypeGlobalLoadingPolicy(ITexture2D::GetTypeId(), E_RESOURCE_LOADING_POLICY::STREAMING);
		mpResourceManagerInstance->RegisterTypeGlobalLoadingPolicy(ICubemapTexture::GetTypeId(), E_RESOURCE_LOADING_POLICY::STREAMING);
		mpResourceManagerInstance->RegisterTypeGlobalLoadingPolicy(IStaticMesh::GetTypeId(), E_RESOURCE_LOADING_POLICY::STREAMING);
		mpResourceManagerInstance->RegisterTypeGlobalLoadingPolicy(ISkinnedMesh::GetTypeId(), E_RESOURCE_LOADING_POLICY::STREAMING);
#endif

		mpResourceManagerInstance->RegisterResourceTypeAlias(ITexture2D::GetTypeId(), IAtlasSubTexture::GetTypeId());
		mpResourceManagerInstance->RegisterResourceTypeAlias(IFont::GetTypeId(), IRuntimeFont::GetTypeId());

		return mpEngineCoreInstance->RegisterSubsystem(DynamicPtrCast<IEngineSubsystem>(mpResourceManagerInstance));
	}

	E_RESULT_CODE CBaseEngineCoreBuilder::_configureJobManager(U32 maxNumOfThreads)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

#if defined (TDE2_USE_WINPLATFORM) || defined (TDE2_USE_UNIXPLATFORM)
		mpJobManagerInstance = TPtr<IJobManager>(CreateBaseJobManager(maxNumOfThreads, result));
#else
#endif

		if (result != RC_OK)
		{
			return result;
		}

		return mpEngineCoreInstance->RegisterSubsystem(DynamicPtrCast<IEngineSubsystem>(mpJobManagerInstance));
	}

	E_RESULT_CODE CBaseEngineCoreBuilder::_configurePluginManager()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

#if defined (TDE2_USE_WINPLATFORM) || defined (TDE2_USE_UNIXPLATFORM)
		mpPluginManagerInstance = TPtr<IPluginManager>(CreateBasePluginManager(mpEngineCoreInstance.Get(), result));
#else
#endif

		if (result != RC_OK)
		{
			return result;
		}

		return mpEngineCoreInstance->RegisterSubsystem(DynamicPtrCast<IEngineSubsystem>(mpPluginManagerInstance));
	}

	E_RESULT_CODE CBaseEngineCoreBuilder::_configureEventManager()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		mpEventManagerInstance = TPtr<IEventManager>(CreateEventManager(result));

		if (result != RC_OK)
		{
			return result;
		}

		return mpEngineCoreInstance->RegisterSubsystem(DynamicPtrCast<IEngineSubsystem>(mpEventManagerInstance));
	}

	E_RESULT_CODE CBaseEngineCoreBuilder::_configureRenderer()
	{
		if (!mIsInitialized || !mpGraphicsContextInstance || !mpResourceManagerInstance)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		IRenderer* pRenderer = CreateForwardRenderer({ mpGraphicsContextInstance, mpResourceManagerInstance, CreateLinearAllocator, nullptr }, result);
		if (result != RC_OK)
		{
			return result;
		}

		auto pFramePostProcessor = TPtr<IFramePostProcessor>(CreateFramePostProcessor({ pRenderer, mpGraphicsContextInstance->GetGraphicsObjectManager(), mpWindowSystemInstance.Get() }, result));
		if (result != RC_OK)
		{
			return result;
		}

		if (pFramePostProcessor)
		{
			const TResourceId defaultProfileResourceId = mpResourceManagerInstance->Load<IPostProcessingProfile>("DefaultResources/Configs/default-profile.camera_profile");
			if (TResourceId::Invalid == defaultProfileResourceId)
			{
				return RC_FILE_NOT_FOUND;
			}

			pFramePostProcessor->SetProcessingProfile(mpResourceManagerInstance->GetResource<IPostProcessingProfile>(defaultProfileResourceId).Get());
		}

		if ((result = pRenderer->SetFramePostProcessor(pFramePostProcessor)) != RC_OK)
		{
			return result;
		}

		return mpEngineCoreInstance->RegisterSubsystem(DynamicPtrCast<IEngineSubsystem>(TPtr<IRenderer>(pRenderer)));
	}

	E_RESULT_CODE CBaseEngineCoreBuilder::_configureInputContext()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

#if defined(TDE2_USE_WINPLATFORM)
		if ((result = mpPluginManagerInstance->LoadPlugin("WindowsInputContext")) != RC_OK)
		{
			return result;
		}
#elif defined(TDE2_USE_UNIXPLATFORM)
		if ((result = mpPluginManagerInstance->LoadPlugin("./UnixInputContext")) != RC_OK)
		{
			return result;
		}
#endif

		return RC_OK;
	}

	E_RESULT_CODE CBaseEngineCoreBuilder::_configureImGUIContext()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

#if defined(TDE2_USE_WINPLATFORM)
		if ((result = mpPluginManagerInstance->LoadPlugin("ImGUIContext")) != RC_OK)
		{
			return result;
		}
#elif defined(TDE2_USE_UNIXPLATFORM)
		if ((result = mpPluginManagerInstance->LoadPlugin("./ImGUIContext")) != RC_OK)
		{
			return result;
		}
#endif

		return RC_OK;
	}

	E_RESULT_CODE CBaseEngineCoreBuilder::_configureEditorsManager()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

#if TDE2_EDITORS_ENABLED
		E_RESULT_CODE result = RC_OK;

		auto pInputContext = mpEngineCoreInstance->GetSubsystem<IInputContext>();
		auto pSceneManager = mpEngineCoreInstance->GetSubsystem<ISceneManager>();

		TPtr<IEditorsManager> pEditorsManager = TPtr<IEditorsManager>(CreateEditorsManager(pInputContext,
			mpEngineCoreInstance->GetSubsystem<IImGUIContext>(),
			mpEventManagerInstance,
			mpEngineCoreInstance->GetWorldInstance(),
			result));

		static const USIZE mainThreadID = std::hash<std::thread::id>{}(std::this_thread::get_id());

		auto pDebugUtility = mpGraphicsContextInstance->GetGraphicsObjectManager()->CreateDebugUtility(mpResourceManagerInstance.Get(), mpEngineCoreInstance->GetSubsystem<IRenderer>().Get()).Get();

		if (auto pSelectionManager = TPtr<ISelectionManager>(CreateSelectionManager(mpResourceManagerInstance, mpWindowSystemInstance, mpGraphicsContextInstance, pEditorsManager.Get(), result)))
		{
			result = result | pEditorsManager->SetSelectionManager(pSelectionManager);
			result = result | mpEngineCoreInstance->GetSubsystem<IRenderer>()->SetSelectionManager(pSelectionManager.Get());

			if (result != RC_OK)
			{
				return result;
			}
		}

		IEditorWindow* pLevelEditorWindow = CreateLevelEditorWindow({ pEditorsManager.Get(), pInputContext.Get(), mpWindowSystemInstance.Get(), pDebugUtility, pSceneManager.Get() }, result);

		std::tuple<std::string, IEditorWindow*> builtinEditors[]
		{
			{ CEditorsManager::mEditorNamesMap.at(E_EDITOR_TYPE::TIME_PROFILER), CreateTimeProfilerEditorWindow(CPerfProfiler::Get(), result) },
			{ CEditorsManager::mEditorNamesMap.at(E_EDITOR_TYPE::MEMORY_PROFILER), CreateMemoryProfilerEditorWindow(DynamicPtrCast<IMemoryProfiler>(CMemoryProfiler::Get()), result) },
			{ CEditorsManager::mEditorNamesMap.at(E_EDITOR_TYPE::LEVEL_EDITOR), pLevelEditorWindow },
			{ CEditorsManager::mEditorNamesMap.at(E_EDITOR_TYPE::DEV_CONSOLE), CreateDevConsoleWindow(result) },
			{ CEditorsManager::mEditorNamesMap.at(E_EDITOR_TYPE::RENDER_TARGET_VIEWER), CreateRenderTargetViewerEditorWindow(mpResourceManagerInstance.Get(), result) },
			{ CEditorsManager::mEditorNamesMap.at(E_EDITOR_TYPE::PROJECT_SETTINGS_EDITOR), CreateProjectSettingsWindow(mpEventManagerInstance, result) },
		};

		dynamic_cast<CTimeProfilerEditorWindow*>(std::get<1>(builtinEditors[0]))->SetMainThreadID(mainThreadID);

		for (auto& currEditorEntry : builtinEditors)
		{
			result = result | pEditorsManager->RegisterEditor(std::get<std::string>(currEditorEntry), std::get<IEditorWindow*>(currEditorEntry));
		}

		if (result != RC_OK)
		{
			return result;
		}

		// \note Register all builtin component's inspectors
		if (RC_OK != (result = CDefaultInspectorsRegistry::RegisterBuiltinInspectors(dynamic_cast<CLevelEditorWindow&>(*pLevelEditorWindow))))
		{
			return result;
		}

		return mpEngineCoreInstance->RegisterSubsystem(DynamicPtrCast<IEngineSubsystem>(pEditorsManager));
#else
		return RC_OK;
#endif
	}

	E_RESULT_CODE CBaseEngineCoreBuilder::_configureSceneManager()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		auto pWorld = TPtr<IWorld>(CreateWorld(mpWindowSystemInstance->GetEventManager(), result));

		// \todo load settings from  settings
		auto pSceneManager = TPtr<ISceneManager>(CreateSceneManager(mpFileSystemInstance, pWorld,
															TPtr<IPrefabsRegistry>(CreatePrefabsRegistry(mpResourceManagerInstance.Get(), mpFileSystemInstance.Get(), pWorld.Get(), result)),
															{ CProjectSettings::Get()->mScenesSettings.mMainLevelScenePath }, result));
		if (result != RC_OK)
		{
			return result;
		}

		return mpEngineCoreInstance->RegisterSubsystem(DynamicPtrCast<IEngineSubsystem>(pSceneManager));
	}

	E_RESULT_CODE CBaseEngineCoreBuilder::_configureLocalizationManager()
	{
		E_RESULT_CODE result = RC_OK;

		// \todo Replace magic constant 
		ILocalizationManager* pLocalizationManager = CreateLocalizationManager(mpFileSystemInstance, mpResourceManagerInstance, CProjectSettings::Get()->mLocalizationSettings, result);
		if (RC_OK != result)
		{
			return result;
		}

		return mpEngineCoreInstance->RegisterSubsystem(DynamicPtrCast<IEngineSubsystem>(TPtr<ILocalizationManager>(pLocalizationManager)));
	}

	E_RESULT_CODE CBaseEngineCoreBuilder::_configureSaveManager()
	{
		E_RESULT_CODE result = RC_OK;

		// \todo Replace magic constant 
		ISaveManager* pSaveManager = CreateSaveManager({ mpFileSystemInstance, "." }, result);
		if (RC_OK != result)
		{
			return result;
		}

		return mpEngineCoreInstance->RegisterSubsystem(DynamicPtrCast<IEngineSubsystem>(TPtr<ISaveManager>(pSaveManager)));
	}


	/*!
		\brief The function loads all user-defined directories and Resources/ alias. All the GAPI dependent stuffs are mounted later
	*/

	static E_RESULT_CODE MountMainDirectories(const TPtr<IFileSystem> pFileSystem)
	{
		E_RESULT_CODE result = RC_OK;

		/// \note Register built-in application's paths
		if ((RC_OK != (result = pFileSystem->MountPhysicalPath(CProjectSettings::Get()->mCommonSettings.mBaseResourcesPath, "Resources/"))) ||
			(RC_OK != (result = pFileSystem->MountPhysicalPath(pFileSystem->CombinePath(CProjectSettings::Get()->mCommonSettings.mBaseResourcesPath, "Engine/"), "DefaultResources/"))))
		{
			return result;
		}

		/// \note Register user's defined aliases for paths
		for (auto&& currUserMountDirectory : CProjectSettings::Get()->mCommonSettings.mAdditionalMountedDirectories)
		{
			if (currUserMountDirectory.mPath.find("Shaders") != std::string::npos) /// \note For shaders directories add graphics context subdirectory to end
			{
				continue;
			}

			result = result | pFileSystem->MountPhysicalPath(currUserMountDirectory.mPath, currUserMountDirectory.mAlias);
		}

		return result;
	}


	static E_RESULT_CODE MountGraphicsDirectories(const TPtr<IFileSystem> pFileSystem, E_GRAPHICS_CONTEXT_GAPI_TYPE type)
	{
		E_RESULT_CODE result = RC_OK;

		static const std::string hlslSubDirectory = "/DX/";
		static const std::string glslSubDirectory = "/GL/";

		static const std::string baseShadersPath   = pFileSystem->CombinePath(pFileSystem->CombinePath(CProjectSettings::Get()->mCommonSettings.mBaseResourcesPath, "Engine/"), "Shaders/");

		std::string baseDefaultShadersPath     = baseShadersPath + "Default";
		std::string basePostEffectsShadersPath = baseShadersPath + "PostEffects";

		switch (type)
		{
			case E_GRAPHICS_CONTEXT_GAPI_TYPE::GCGT_DIRECT3D11:
				baseDefaultShadersPath.append(hlslSubDirectory);
				basePostEffectsShadersPath.append(hlslSubDirectory);
				break;
			case E_GRAPHICS_CONTEXT_GAPI_TYPE::GCGT_OPENGL3X:
				baseDefaultShadersPath.append(glslSubDirectory);
				basePostEffectsShadersPath.append(glslSubDirectory);
				break;
			default:
				if (static_cast<E_PARAMETERS>(CProjectSettings::Get()->mCommonSettings.mFlags) & E_PARAMETERS::P_WINDOWLESS_MODE)
				{
				}
				else
				{
					TDE2_UNREACHABLE();
				}

				break;
		}

		/// \note Register built-in application's paths
		if ((RC_OK != (result = pFileSystem->MountPhysicalPath(baseDefaultShadersPath, "Shaders/Default/"))) ||
			(RC_OK != (result = pFileSystem->MountPhysicalPath(basePostEffectsShadersPath, "Shaders/PostEffects/"))) ||
			(RC_OK != (result = pFileSystem->MountPhysicalPath(baseShadersPath, "Shaders/", 1))))
		{
			return result;
		}

		/// \note Register user's defined aliases for paths (graphics only)
		for (auto&& currUserMountDirectory : CProjectSettings::Get()->mCommonSettings.mAdditionalMountedDirectories)
		{
			if (currUserMountDirectory.mPath.find("Shaders") == std::string::npos) /// \note At this stage register only graphics related paths
			{
				continue;
			}

			const bool pathEndsWithSeparator = Wrench::StringUtils::EndsWith(currUserMountDirectory.mPath, "/");

			switch (type)
			{
				case E_GRAPHICS_CONTEXT_GAPI_TYPE::GCGT_DIRECT3D11:
					currUserMountDirectory.mPath.append(pathEndsWithSeparator ? hlslSubDirectory.substr(1) : hlslSubDirectory);
					break;
				case E_GRAPHICS_CONTEXT_GAPI_TYPE::GCGT_OPENGL3X:
					currUserMountDirectory.mPath.append(pathEndsWithSeparator ? glslSubDirectory.substr(1) : glslSubDirectory);
					break;
				default:
					TDE2_UNREACHABLE();
					break;
			}

			result = result | pFileSystem->MountPhysicalPath(currUserMountDirectory.mPath, currUserMountDirectory.mAlias);
		}

		return result;
	}


	static E_RESULT_CODE LoadUserPlugins(TPtr<IPluginManager> pPluginsManager, const std::vector<std::string>& pluginsIdentifiers)
	{
		E_RESULT_CODE result = RC_OK;

		/// pluginId is a filename of a shared library, *.dll or *.so files
		for (const std::string& pluginId : pluginsIdentifiers)
		{
			result = pPluginsManager->LoadPlugin(pluginId);
			if (RC_OK != result)
			{
				LOG_WARNING(Wrench::StringUtils::Format("[LoadUserPlugins] ({0}) plugin's loading failed", pluginId));
			}
		}

		return result;
	}


	static E_RESULT_CODE ConfigureGameModesManager(TPtr<IEngineCore> pEngineCore)
	{
		E_RESULT_CODE result = RC_OK;

		auto pSubsystem = TPtr<IGameModesManager>(CreateGameModesManager(result));
		if (!pSubsystem || RC_OK != result)
		{
			return result;
		}

		/// \note Set the default game mode
		result = result | pSubsystem->SwitchMode(TPtr<IGameMode>(CreateBaseGameMode(pSubsystem.Get(), result)));
		TDE2_ASSERT(RC_OK == result);

		/// \note Register the splash screen if it's enabled
		if (CProjectSettings::Get()->mSplashScreenSettings.mIsEnabled)
		{
			result = result | pSubsystem->PushMode(TPtr<IGameMode>(CreateSplashScreenGameMode(
																		pSubsystem.Get(),
																		pEngineCore->GetSubsystem<ISceneManager>(),
																		{
																			CProjectSettings::Get()->mSplashScreenSettings.mShowDuration
																		},
																		result)));
			TDE2_ASSERT(RC_OK == result);
		}

		return pEngineCore->RegisterSubsystem(DynamicPtrCast<IEngineSubsystem>(pSubsystem));
	}


	TPtr<IEngineCore> CBaseEngineCoreBuilder::GetEngineCore()
	{
		PANIC_ON_FAILURE(_configureFileSystem());
		PANIC_ON_FAILURE(_initEngineSettings());

		PANIC_ON_FAILURE(MountMainDirectories(mpFileSystemInstance));

		const bool isWindowModeEnabled = !(static_cast<E_PARAMETERS>(CProjectSettings::Get()->mCommonSettings.mFlags) & E_PARAMETERS::P_WINDOWLESS_MODE);

		PANIC_ON_FAILURE(_configureJobManager(CProjectSettings::Get()->mCommonSettings.mMaxNumOfWorkerThreads));
		PANIC_ON_FAILURE(_configureEventManager());
		PANIC_ON_FAILURE(_configureResourceManager());

		PANIC_ON_FAILURE(_configureWindowSystem(
			CProjectSettings::Get()->mCommonSettings.mApplicationName,
			CGameUserSettings::Get()->mWindowWidth,
			CGameUserSettings::Get()->mWindowHeight,
			CProjectSettings::Get()->mCommonSettings.mFlags));

		PANIC_ON_FAILURE(_configurePluginManager());
		PANIC_ON_FAILURE(_configureGraphicsContext(isWindowModeEnabled ? CProjectSettings::Get()->mGraphicsSettings.mRendererPluginFilePath : Wrench::StringUtils::GetEmptyStr()));

		PANIC_ON_FAILURE(MountGraphicsDirectories(mpFileSystemInstance, mpGraphicsContextInstance->GetContextInfo().mGapiType));

		if (isWindowModeEnabled) { _configureAudioContext(CProjectSettings::Get()->mAudioSettings.mAudioPluginFilePath); }
		if (isWindowModeEnabled) { PANIC_ON_FAILURE(_configureInputContext()); }
		
		PANIC_ON_FAILURE(_configureSceneManager());

		mpFileSystemInstance->SetJobManager(mpJobManagerInstance.Get());
		LOG_MESSAGE(std::string("[ConfigFIleEngineCoreBuilder] Async file I/O operations status: ").append(mpFileSystemInstance->IsStreamingEnabled() ? "enabled" : "disabled"));

		E_RESULT_CODE result = _registerBuiltinInfrastructure(isWindowModeEnabled);

		defer([this] /// \note Should release this refs to provide correct release of the memory when the app will be terminated
		{
			mpWindowSystemInstance = nullptr;
			mpJobManagerInstance = nullptr;
			mpPluginManagerInstance = nullptr;
			mpGraphicsContextInstance = nullptr;
			mpFileSystemInstance = nullptr;
			mpResourceManagerInstance = nullptr;
			mpEventManagerInstance = nullptr;
			mpEngineCoreInstance = nullptr;
		});

		if (result != RC_OK)
		{
			LOG_WARNING("[ConfigFIleEngineCoreBuilder] Couldn't register built-in types for the resource manager");

			return mpEngineCoreInstance;
		}

		PANIC_ON_FAILURE(_configureLocalizationManager());
		PANIC_ON_FAILURE(_configureSaveManager());
		
		if (isWindowModeEnabled)
		{
			PANIC_ON_FAILURE(_configureRenderer());
			PANIC_ON_FAILURE(_configureImGUIContext());
			PANIC_ON_FAILURE(_configureEditorsManager());
		}

		PANIC_ON_FAILURE(ConfigureGameModesManager(mpEngineCoreInstance));

		// \note Try to load user defined plugins, if some plugin cannot be bound it's not failure we just won't get some extra functionality 
		LoadUserPlugins(mpPluginManagerInstance, CProjectSettings::Get()->mCommonSettings.mUserPluginsToLoad);		

		return mpEngineCoreInstance;
	}

#if TDE2_EDITORS_ENABLED

	void CBaseEngineCoreBuilder::RegisterObjectInProfiler(const std::string& id)
	{
	}
		
	void CBaseEngineCoreBuilder::OnBeforeMemoryRelease()
	{
	}

#endif

	E_RESULT_CODE CBaseEngineCoreBuilder::_registerBuiltinInfrastructure(bool isWindowModeEnabled)
	{
		if (!mpResourceManagerInstance)
		{
			return RC_FAIL;
		}

		auto registerResourceType = [](IResourceManager* pResourceMgr, IResourceLoader* pLoader, IResourceFactory* pFactory) -> E_RESULT_CODE
		{
			/// \note register a loader
			TResult<TResourceLoaderId> loaderRegistrationResult = pResourceMgr->RegisterLoader(pLoader);

			if (loaderRegistrationResult.HasError())
			{
				return loaderRegistrationResult.GetError();
			};

			/// \note register resource's factory
			TResult<TResourceFactoryId> factoryRegistrationResult = pResourceMgr->RegisterFactory(pFactory);

			if (factoryRegistrationResult.HasError())
			{
				return factoryRegistrationResult.GetError();
			};

			return RC_OK;
		};

		/// Register builtin factories for IResourceManager
		using ResourceLoaderFactoryFunctor = std::function<IResourceLoader*(IResourceManager*, IGraphicsContext*, IFileSystem*, E_RESULT_CODE&)>;
		using ResourceFactoryFactoryFunctor = std::function<IResourceFactory*(IResourceManager*, IGraphicsContext*, E_RESULT_CODE&)>;

		/// The third parameter defines whether the resource needs window mode or not
		std::tuple<ResourceLoaderFactoryFunctor, ResourceFactoryFactoryFunctor, bool> builtinResourcesConstructorsTable[]
		{
			{ CreateBaseMaterialLoader, CreateBaseMaterialFactory, true },
			{ CreateTextureAtlasLoader, CreateTextureAtlasFactory, true },
			{ CreateAtlasSubTextureLoader, CreateAtlasSubTextureFactory, true },
			{ CreateStaticMeshLoader, CreateStaticMeshFactory, true },
			{ CreateSkinnedMeshLoader, CreateSkinnedMeshFactory, true },
			{ CreateBasePostProcessingProfileLoader, CreateBasePostProcessingProfileFactory, false },
			{ CreateAnimationClipLoader, CreateAnimationClipFactory, false },
			{ CreateParticleEffectLoader, CreateParticleEffectFactory, true },
			{ CreateSkeletonLoader, CreateSkeletonFactory, false },
		};

		/// create material loader
		E_RESULT_CODE result = RC_OK;

		IResourceLoader* pResourceLoader = nullptr;
		IResourceFactory* pResourceFactory = nullptr;

		for (auto&& currResourceConstructors : builtinResourcesConstructorsTable)
		{
			if (!isWindowModeEnabled && std::get<bool>(currResourceConstructors) != isWindowModeEnabled)
			{
				continue;
			}

			pResourceLoader = std::get<ResourceLoaderFactoryFunctor>(currResourceConstructors)(mpResourceManagerInstance.Get(), mpGraphicsContextInstance.Get(), mpFileSystemInstance.Get(), result);

			if (result != RC_OK)
			{
				return result;
			}

			/// create a factory
			pResourceFactory = std::get<ResourceFactoryFactoryFunctor>(currResourceConstructors)(mpResourceManagerInstance.Get(), mpGraphicsContextInstance.Get(), result);

			if (result != RC_OK)
			{
				return result;
			}

			/// \note register a resource type
			if ((result = registerResourceType(mpResourceManagerInstance.Get(), pResourceLoader, pResourceFactory)) != RC_OK)
			{
				return result;
			}
		}

		/// Register another resources which use another signatures of their factory methods
		using ResourceLoaderFactoryFunctor2 = std::function<IResourceLoader* (IResourceManager*, IFileSystem*, E_RESULT_CODE&)>;
		using ResourceFactoryFactoryFunctor2 = std::function<IResourceFactory* (IResourceManager*, E_RESULT_CODE&)>;

		std::tuple<ResourceLoaderFactoryFunctor2, ResourceFactoryFactoryFunctor2, bool> builtinResourcesConstructorsTable2[]
		{
			{ CreateFontLoader, CreateFontFactory, true },
			{ CreateLocalizationPackageLoader, CreateLocalizationPackageFactory, true },
			{ CreatePrefabsManifestLoader, CreatePrefabsManifestFactory, false },
		};

		for (auto&& currResourceConstructors : builtinResourcesConstructorsTable2)
		{
			if (!isWindowModeEnabled && std::get<bool>(currResourceConstructors) != isWindowModeEnabled)
			{
				continue;
			}

			pResourceLoader = std::get<ResourceLoaderFactoryFunctor2>(currResourceConstructors)(mpResourceManagerInstance.Get(), mpFileSystemInstance.Get(), result);

			if (result != RC_OK)
			{
				return result;
			}

			/// create a factory
			pResourceFactory = std::get<ResourceFactoryFactoryFunctor2>(currResourceConstructors)(mpResourceManagerInstance.Get(), result);

			if (result != RC_OK)
			{
				return result;
			}

			/// \note register a resource type
			if ((result = registerResourceType(mpResourceManagerInstance.Get(), pResourceLoader, pResourceFactory)) != RC_OK)
			{
				return result;
			}
		}

		/// \note register runtime font's resource type
		pResourceLoader = CreateRuntimeFontLoader(mpResourceManagerInstance.Get(), mpFileSystemInstance.Get(), result);

		if (result != RC_OK)
		{
			return result;
		}

		pResourceFactory = CreateRuntimeFontFactory(mpResourceManagerInstance.Get(), mpFileSystemInstance.Get(), result);

		if (result != RC_OK)
		{
			return result;
		}

		if ((result = registerResourceType(mpResourceManagerInstance.Get(), pResourceLoader, pResourceFactory)) != RC_OK)
		{
			return result;
		}

		/// \todo Move this into another place later
		TRuntimeFontParameters fontParams;
		fontParams.mTrueTypeFontFilePath = "DefaultResources/Fonts/OpenSans-Regular.ttf";

		mpResourceManagerInstance->Create<IRuntimeFont>("OpenSans.font", fontParams);

		/// \note Load default prefabs collection
		mpResourceManagerInstance->Load<IPrefabsManifest>(CProjectSettings::Get()->mCommonSettings.mPathToDefaultPrefabsManifest);

		return RC_OK;
	}
}