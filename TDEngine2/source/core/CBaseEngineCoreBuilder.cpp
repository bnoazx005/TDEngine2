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

	E_RESULT_CODE CBaseEngineCoreBuilder::_configureGraphicsContext(E_GRAPHICS_CONTEXT_GAPI_TYPE type)
	{
		if (!mIsInitialized || !mpPluginManagerInstance)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		switch (type)
		{
	#if defined(TDE2_USE_WINPLATFORM)
			case GCGT_DIRECT3D11:																	/// try to create D3D11 Graphics context
				if ((result = mpPluginManagerInstance->LoadPlugin("D3D11GraphicsContext")) != RC_OK)
				{
					return result;
				}
				break;
	#elif defined(TDE2_USE_UNIXPLATFORM)
			case GCGT_DIRECT3D11:
				TDE2_UNREACHABLE();
				break;
	#endif
			case GCGT_OPENGL3X:																		/// try to create OGL 3.X Graphics context
	#if defined (TDE2_USE_WINPLATFORM)
				result = mpPluginManagerInstance->LoadPlugin("GLGraphicsContext");
	#elif defined (TDE2_USE_UNIXPLATFORM)
				result = mpPluginManagerInstance->LoadPlugin("./GLGraphicsContext");
	#else
	#endif

				if (result != RC_OK)
				{
					return result;
				}
				break;
			
			case GCGT_UNKNOWN: 
				mpGraphicsContextInstance = CreateProxyGraphicsContext(mpWindowSystemInstance, result);
				if (RC_OK != result)
				{
					return result;
				}

				result = mpEngineCoreInstance->RegisterSubsystem(DynamicPtrCast<IEngineSubsystem>(mpGraphicsContextInstance));
				return result;

			default:
				TDE2_UNREACHABLE();
				return RC_FAIL;
		}

		mpGraphicsContextInstance = mpEngineCoreInstance->GetSubsystem<IGraphicsContext>();

		return RC_OK;
	}

	E_RESULT_CODE CBaseEngineCoreBuilder::_configureAudioContext(E_AUDIO_CONTEXT_API_TYPE type)
	{
		if (!mIsInitialized || !mpPluginManagerInstance)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		switch (type)
		{
			case E_AUDIO_CONTEXT_API_TYPE::FMOD:
#if defined (TDE2_USE_WINPLATFORM)
			result = mpPluginManagerInstance->LoadPlugin("FmodAudioContext");
#elif defined (TDE2_USE_UNIXPLATFORM)
			result = mpPluginManagerInstance->LoadPlugin("./FmodAudioContext");
#else
#endif

			if (result != RC_OK)
			{
				LOG_ERROR("[CBaseEngineCoreBuilder] FmodAudioContext's loading failed. Audio components won't be available at runtime");
				return result;
			}
			break;
			default:
				return RC_FAIL;
		}

		return RC_OK;
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

	E_RESULT_CODE CBaseEngineCoreBuilder::_configureResourceManager()
	{
		if (!mIsInitialized || !mpJobManagerInstance || !mpFileSystemInstance)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		mpResourceManagerInstance = TPtr<IResourceManager>(CreateResourceManager(mpJobManagerInstance, result));

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
			const TResourceId defaultProfileResourceId = mpResourceManagerInstance->Load<IPostProcessingProfile>("DefaultConfigs/default-profile.camera_profile");
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
		if (RC_OK != (result = CDefeaultInspectorsRegistry::RegisterBuiltinInspectors(dynamic_cast<CLevelEditorWindow&>(*pLevelEditorWindow))))
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

		// \todo load settings from  settings
		auto pSceneManager = TPtr<ISceneManager>(CreateSceneManager(mpFileSystemInstance, TPtr<IWorld>(CreateWorld(mpWindowSystemInstance->GetEventManager(), result)), {}, result));
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

	TPtr<IEngineCore> CBaseEngineCoreBuilder::GetEngineCore()
	{
		PANIC_ON_FAILURE(_configureFileSystem());
		PANIC_ON_FAILURE(_initEngineSettings());

		const bool isWindowModeEnabled = !(static_cast<E_PARAMETERS>(CProjectSettings::Get()->mCommonSettings.mFlags) & E_PARAMETERS::P_WINDOWLESS_MODE);

		PANIC_ON_FAILURE(_mountDirectories(CProjectSettings::Get()->mGraphicsSettings.mGraphicsContextType));

		PANIC_ON_FAILURE(_configureJobManager(CProjectSettings::Get()->mCommonSettings.mMaxNumOfWorkerThreads));
		PANIC_ON_FAILURE(_configureEventManager());
		PANIC_ON_FAILURE(_configureResourceManager());

		PANIC_ON_FAILURE(_configureWindowSystem(
			CProjectSettings::Get()->mCommonSettings.mApplicationName,
			CGameUserSettings::Get()->mWindowWidth,
			CGameUserSettings::Get()->mWindowHeight,
			CProjectSettings::Get()->mCommonSettings.mFlags));

		PANIC_ON_FAILURE(_configurePluginManager());
		PANIC_ON_FAILURE(_configureGraphicsContext(isWindowModeEnabled ? CProjectSettings::Get()->mGraphicsSettings.mGraphicsContextType : E_GRAPHICS_CONTEXT_GAPI_TYPE::GCGT_UNKNOWN));
		
		if (isWindowModeEnabled) { _configureAudioContext(CProjectSettings::Get()->mAudioSettings.mAudioContextType); }
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

	E_RESULT_CODE CBaseEngineCoreBuilder::_mountDirectories(E_GRAPHICS_CONTEXT_GAPI_TYPE type)
	{
		E_RESULT_CODE result = RC_OK;

		static const std::string hslsSubDirectory = "/DX/";
		static const std::string gslsSubDirectory = "/GL/";

#if TDE2_PRODUCTION_MODE
		static const std::string baseResourcesPath = "../../Resources/";
#else
		static const std::string baseResourcesPath = "../../Resources/";
		TDE2_UNIMPLEMENTED();
#endif

		static const std::string baseShadersPath = baseResourcesPath + "Shaders/";
		static const std::string baseMaterialsPath = baseResourcesPath + "Materials/";
		static const std::string baseConfigsPath = baseResourcesPath + "Configs/";
		static const std::string baseFontsPath = baseResourcesPath + "Fonts/";

		std::string baseDefaultShadersPath = baseShadersPath + "Default";
		std::string basePostEffectsShadersPath = baseShadersPath + "PostEffects";

		switch (type)
		{
			case E_GRAPHICS_CONTEXT_GAPI_TYPE::GCGT_DIRECT3D11:
				baseDefaultShadersPath.append(hslsSubDirectory);
				basePostEffectsShadersPath.append(hslsSubDirectory);
				break;
			case E_GRAPHICS_CONTEXT_GAPI_TYPE::GCGT_OPENGL3X:
				baseDefaultShadersPath.append(gslsSubDirectory);
				basePostEffectsShadersPath.append(gslsSubDirectory);
				break;
			default:
				TDE2_UNREACHABLE();
				break;
		}

		if ((RC_OK != (result = mpFileSystemInstance->MountPhysicalPath(baseResourcesPath, "Resources/"))) || 
			(RC_OK != (result = mpFileSystemInstance->MountPhysicalPath(baseConfigsPath, "DefaultConfigs/"))) ||
			(RC_OK != (result = mpFileSystemInstance->MountPhysicalPath(baseMaterialsPath, "DefaultMaterials/"))) ||
			(RC_OK != (result = mpFileSystemInstance->MountPhysicalPath(baseFontsPath, "DefaultFonts/"))) ||
			(RC_OK != (result = mpFileSystemInstance->MountPhysicalPath(baseDefaultShadersPath, "Shaders/Default/"))) ||
			(RC_OK != (result = mpFileSystemInstance->MountPhysicalPath(basePostEffectsShadersPath, "Shaders/PostEffects/"))) ||
			(RC_OK != (result = mpFileSystemInstance->MountPhysicalPath(baseShadersPath, "Shaders/", 1))))
		{
			return result;
		}

		return RC_OK;
	}

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

		/// \note register font's resource type
		pResourceLoader = CreateFontLoader(mpResourceManagerInstance.Get(), mpFileSystemInstance.Get(), result);

		if (result != RC_OK)
		{
			return result;
		}

		pResourceFactory = CreateFontFactory(mpResourceManagerInstance.Get(), result);

		if (result != RC_OK)
		{
			return result;
		}

		if ((result = registerResourceType(mpResourceManagerInstance.Get(), pResourceLoader, pResourceFactory)) != RC_OK)
		{
			return result;
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

		/// \note register localization package's resource type
		pResourceLoader = CreateLocalizationPackageLoader(mpResourceManagerInstance.Get(), mpFileSystemInstance.Get(), result);

		if (result != RC_OK)
		{
			return result;
		}

		pResourceFactory = CreateLocalizationPackageFactory(mpResourceManagerInstance.Get(), result);

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
		fontParams.mTrueTypeFontFilePath = "DefaultFonts/OpenSans-Regular.ttf";

		mpResourceManagerInstance->Create<IRuntimeFont>("OpenSans.font", fontParams);

		return RC_OK;
	}
}