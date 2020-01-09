#include "./../../include/core/CConfigFileEngineCoreBuilder.h"
#include "./../../include/core/CEngineCore.h"
#include "./../../include/core/IPlugin.h"
#include "./../../include/core/IFileSystem.h"
#include "./../../include/core/CResourceManager.h"
#include "./../../include/core/CBaseJobManager.h"
#include "./../../include/core/IJobManager.h"
#include "./../../include/core/CBasePluginManager.h"
#include "./../../include/core/CEventManager.h"
#include "./../../include/core/memory/CMemoryManager.h"
#include "./../../include/core/memory/CLinearAllocator.h"
#include "./../../include/core/memory/CPoolAllocator.h"
#include "./../../include/core/memory/CStackAllocator.h"
#include "./../../include/core/IInputContext.h"
#include "./../../include/core/IGraphicsContext.h"
#include "./../../include/core/CFont.h"
#include "./../../include/core/IImGUIContext.h"
#include "./../../include/platform/win32/CWin32WindowSystem.h"
#include "./../../include/platform/win32/CWin32FileSystem.h"
#include "./../../include/platform/unix/CUnixWindowSystem.h"
#include "./../../include/platform/unix/CUnixFileSystem.h"
#include "./../../include/platform/CTextFileReader.h"
#include "./../../include/platform/CConfigFileReader.h"
#include "./../../include/platform/CBinaryFileReader.h"
#include "./../../include/platform/CBinaryFileWriter.h"
#include "./../../include/platform/CImageFileWriter.h"
#include "./../../include/platform/CYAMLFile.h"
#include "./../../include/platform/CBinaryMeshFileReader.h"
#include "./../../include/graphics/CForwardRenderer.h"
#include "./../../include/graphics/CBaseMaterial.h"
#include "./../../include/graphics/CTextureAtlas.h"
#include "./../../include/utils/CFileLogger.h"
#include "./../../include/graphics/CStaticMesh.h"
#include "./../../include/editor/CEditorsManager.h"
#include <memory>
#include <thread>
#include <functional>


namespace TDEngine2
{
	CConfigFileEngineCoreBuilder::CConfigFileEngineCoreBuilder() :
		mIsInitialized(false), 
		mpEngineCoreInstance(nullptr), 
		mpWindowSystemInstance(nullptr), 
		mpJobManagerInstance(nullptr),
		mpPluginManagerInstance(nullptr), 
		mpGraphicsContextInstance(nullptr), 
		mpResourceManagerInstance(nullptr)
	{
	}

	/*!
		\brief The method initialized the builder's object

		\param[in] A callback to a factory's function of IEngineCore's objects

		\return RC_OK if everything went ok, or some other code, which describes an error
	*/

	E_RESULT_CODE CConfigFileEngineCoreBuilder::Init(TCreateEngineCoreCallback pEngineCoreFactoryCallback, const std::string& configFilename)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pEngineCoreFactoryCallback || configFilename.empty())
		{
			return RC_INVALID_ARGS;
		}
		
		E_RESULT_CODE result = RC_OK;

		mConfigFilename = configFilename;

		mpEngineCoreInstance = pEngineCoreFactoryCallback(result);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CConfigFileEngineCoreBuilder::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CConfigFileEngineCoreBuilder::_configureGraphicsContext(E_GRAPHICS_CONTEXT_GAPI_TYPE type)
	{
		if (!mIsInitialized || !mpPluginManagerInstance)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		switch (type)
		{
#if defined(TDE2_USE_WIN32PLATFORM)
		case GCGT_DIRECT3D11:																	/// try to create D3D11 Graphics context
			if ((result = mpPluginManagerInstance->LoadPlugin("D3D11GraphicsContext")) != RC_OK)
			{
				return result;
			}
			break;
#endif
		case GCGT_OPENGL3X:																		/// try to create OGL 3.X Graphics context
#if defined (TDE2_USE_WIN32PLATFORM)
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
		default:
			return RC_FAIL;
		}

		mpGraphicsContextInstance = mpEngineCoreInstance->GetSubsystem<IGraphicsContext>();

		return RC_OK;
	}

	E_RESULT_CODE CConfigFileEngineCoreBuilder::_configureWindowSystem(const std::string& name, U32 width, U32 height, U32 flags)
	{
		if (!mIsInitialized || !mpEventManagerInstance)
		{
			return RC_FAIL;
		}

		mpWindowSystemInstance = nullptr;

		E_RESULT_CODE result = RC_OK;

#if defined (TDE2_USE_WIN32PLATFORM)																/// Win32 Platform
		mpWindowSystemInstance = CreateWin32WindowSystem(mpEventManagerInstance, name, width, height, flags, result);
#elif defined (TDE2_USE_UNIXPLATFORM)
		mpWindowSystemInstance = CreateUnixWindowSystem(mpEventManagerInstance, name, width, height, flags, result);
#else
#endif

		if (result != RC_OK)
		{
			return result;
		}

		return mpEngineCoreInstance->RegisterSubsystem(mpWindowSystemInstance);
	}

	E_RESULT_CODE CConfigFileEngineCoreBuilder::_configureFileSystem()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

#if defined (TDE2_USE_WIN32PLATFORM)
		mpFileSystemInstance = CreateWin32FileSystem(result);
#elif defined (TDE2_USE_UNIXPLATFORM)
		mpFileSystemInstance = CreateUnixFileSystem(result);
#else
#endif

		/// register known file types factories
		if (((result = mpFileSystemInstance->RegisterFileFactory<ITextFileReader>(CreateTextFileReader)) != RC_OK) ||
			((result = mpFileSystemInstance->RegisterFileFactory<IConfigFileReader>(CreateConfigFileReader)) != RC_OK) ||
			((result = mpFileSystemInstance->RegisterFileFactory<IBinaryFileReader>(CreateBinaryFileReader)) != RC_OK) ||
			((result = mpFileSystemInstance->RegisterFileFactory<IBinaryFileWriter>(CreateBinaryFileWriter)) != RC_OK) ||
			((result = mpFileSystemInstance->RegisterFileFactory<IImageFileWriter>(CreateImageFileWriter)) != RC_OK)   ||
			((result = mpFileSystemInstance->RegisterFileFactory<IYAMLFileReader>(CreateYAMLFileReader)) != RC_OK)     ||
			((result = mpFileSystemInstance->RegisterFileFactory<IYAMLFileWriter>(CreateYAMLFileWriter)) != RC_OK)     ||
			((result = mpFileSystemInstance->RegisterFileFactory<IBinaryMeshFileReader>(CreateBinaryMeshFileReader)) != RC_OK))
		{
			return result;
		}

		if ((result = mpFileSystemInstance->Mount("./../../Resources/Shaders/", "Shaders")) != RC_OK)
		{
			return result;
		}

		return mpEngineCoreInstance->RegisterSubsystem(dynamic_cast<IEngineSubsystem*>(mpFileSystemInstance));
	}

	E_RESULT_CODE CConfigFileEngineCoreBuilder::_configureResourceManager()
	{
		if (!mIsInitialized || !mpJobManagerInstance || !mpFileSystemInstance)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		mpResourceManagerInstance = CreateResourceManager(mpJobManagerInstance, result);

		if (result != RC_OK)
		{
			return result;
		}

		return mpEngineCoreInstance->RegisterSubsystem(mpResourceManagerInstance);
	}

	E_RESULT_CODE CConfigFileEngineCoreBuilder::_configureJobManager(U32 maxNumOfThreads)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

#if defined (TDE2_USE_WIN32PLATFORM) || defined (TDE2_USE_UNIXPLATFORM)
		IEngineSubsystem* pJobManager = CreateBaseJobManager(maxNumOfThreads, result);
#else
#endif

		if (result != RC_OK)
		{
			return result;
		}

		mpJobManagerInstance = dynamic_cast<IJobManager*>(pJobManager);

		return mpEngineCoreInstance->RegisterSubsystem(pJobManager);
	}

	E_RESULT_CODE CConfigFileEngineCoreBuilder::_configurePluginManager()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

#if defined (TDE2_USE_WIN32PLATFORM) || defined (TDE2_USE_UNIXPLATFORM)
		IEngineSubsystem* pPluginManager = CreateBasePluginManager(mpEngineCoreInstance, result);
#else
#endif

		if (result != RC_OK)
		{
			return result;
		}

		mpPluginManagerInstance = dynamic_cast<IPluginManager*>(pPluginManager);

		return mpEngineCoreInstance->RegisterSubsystem(pPluginManager);
	}

	E_RESULT_CODE CConfigFileEngineCoreBuilder::_configureEventManager()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		mpEventManagerInstance = CreateEventManager(result);

		if (result != RC_OK)
		{
			return result;
		}

		return mpEngineCoreInstance->RegisterSubsystem(mpEventManagerInstance);
	}

	E_RESULT_CODE CConfigFileEngineCoreBuilder::_configureRenderer()
	{
		if (!mIsInitialized || !mpMemoryManagerInstance || !mpGraphicsContextInstance || !mpResourceManagerInstance)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		IAllocator* pAllocator = mpMemoryManagerInstance->CreateAllocator<CStackAllocator>(static_cast<U32>(NumOfRenderQueuesGroup + 1) * PerRenderQueueMemoryBlockSize, "Renderer");

		IEngineSubsystem* pRenderer = CreateForwardRenderer(mpGraphicsContextInstance, mpResourceManagerInstance, pAllocator, result);

		if (result != RC_OK)
		{
			return result;
		}

		return mpEngineCoreInstance->RegisterSubsystem(pRenderer);
	}

	E_RESULT_CODE CConfigFileEngineCoreBuilder::_configureMemoryManager(U32 totalMemorySize)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		mpMemoryManagerInstance = CreateMemoryManager(totalMemorySize, result);

		if (result != RC_OK)
		{
			return result;
		}

		/// register built in factories

		auto allocatorFactoriesCallback =
		{
			CreateLinearAllocatorFactory,
			CreateStackAllocatorFactory,
			CreatePoolAllocatorFactory
		};

		IAllocatorFactory* pAllocatorFactory = nullptr;

		for (auto currFactoryCallback : allocatorFactoriesCallback)
		{
			pAllocatorFactory = currFactoryCallback(result);

			if (result != RC_OK)
			{
				return result;
			}

			if ((result = mpMemoryManagerInstance->RegisterFactory(pAllocatorFactory)) != RC_OK)
			{
				return result;
			}
		}

		return mpEngineCoreInstance->RegisterSubsystem(mpMemoryManagerInstance);
	}

	E_RESULT_CODE CConfigFileEngineCoreBuilder::_configureInputContext()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

#if defined(TDE2_USE_WIN32PLATFORM)
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

	E_RESULT_CODE CConfigFileEngineCoreBuilder::_configureImGUIContext()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

#if defined(TDE2_USE_WIN32PLATFORM)
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

	E_RESULT_CODE CConfigFileEngineCoreBuilder::_configureEditorsManager()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

#if TDE2_EDITORS_ENABLED
		E_RESULT_CODE result = RC_OK;
		IEditorsManager* pEditorsManager = CreateEditorsManager(mpEngineCoreInstance->GetSubsystem<IInputContext>(), 
																mpEngineCoreInstance->GetSubsystem<IImGUIContext>(), 
																result);
#else
		return RC_OK;
#endif

		if (result != RC_OK)
		{
			return result;
		}

		return mpEngineCoreInstance->RegisterSubsystem(pEditorsManager);
	}

	IEngineCore* CConfigFileEngineCoreBuilder::GetEngineCore()
	{
		PANIC_ON_FAILURE(_configureFileSystem());

		// read config file
		TResult<TEngineSettings> settingsResult = _readConfigurationFile(mpFileSystemInstance, mConfigFilename);

		if (settingsResult.HasError())
		{
			Panic("[ConfigFIleEngineCoreBuilder] Can't read engine settings. Error code: " + settingsResult.GetError());

			return nullptr;
		}

		TEngineSettings engineSettings = settingsResult.Get();

		PANIC_ON_FAILURE(_configureJobManager(engineSettings.mMaxNumOfWorkerThreads));
		PANIC_ON_FAILURE(_configureMemoryManager(engineSettings.mTotalPreallocatedMemorySize));
		PANIC_ON_FAILURE(_configureEventManager());
		PANIC_ON_FAILURE(_configureResourceManager());
		PANIC_ON_FAILURE(_configureWindowSystem(engineSettings.mApplicationName, engineSettings.mWindowWidth, engineSettings.mWindowHeight, engineSettings.mFlags));
		PANIC_ON_FAILURE(_configurePluginManager());
		PANIC_ON_FAILURE(_configureGraphicsContext(engineSettings.mGraphicsContextType));
		PANIC_ON_FAILURE(_configureInputContext());

		mpFileSystemInstance->SetJobManager(mpJobManagerInstance);
		LOG_MESSAGE(std::string("[ConfigFIleEngineCoreBuilder] Async file I/O operations status: ").append(mpFileSystemInstance->IsStreamingEnabled() ? "enabled" : "disabled"));

		E_RESULT_CODE result = _registerBuiltinInfrastructure();

		if (result != RC_OK)
		{
			LOG_WARNING("[ConfigFIleEngineCoreBuilder] Couldn't register built-in types for the resource manager");

			return mpEngineCoreInstance;
		}

		PANIC_ON_FAILURE(_configureRenderer());
		PANIC_ON_FAILURE(_configureImGUIContext());
		PANIC_ON_FAILURE(_configureEditorsManager());

		return mpEngineCoreInstance;
	}

	E_RESULT_CODE CConfigFileEngineCoreBuilder::_registerBuiltinInfrastructure()
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

		/// create material loader
		E_RESULT_CODE result = RC_OK;

		IResourceLoader* pResourceLoader = CreateBaseMaterialLoader(mpResourceManagerInstance, mpGraphicsContextInstance, mpFileSystemInstance, result);

		if (result != RC_OK)
		{
			return result;
		}

		/// create material factory
		IResourceFactory* pResourceFactory = CreateBaseMaterialFactory(mpResourceManagerInstance, mpGraphicsContextInstance, result);

		if (result != RC_OK)
		{
			return result;
		}

		/// \note register a material type
		if ((result = registerResourceType(mpResourceManagerInstance, pResourceLoader, pResourceFactory)) != RC_OK)
		{
			return result;
		}

		/// \note register texture atlas's infrastructure
		pResourceLoader = CreateTextureAtlasLoader(mpResourceManagerInstance, mpGraphicsContextInstance, mpFileSystemInstance, result);

		if (result != RC_OK)
		{
			return result;
		}

		pResourceFactory = CreateTextureAtlasFactory(mpResourceManagerInstance, mpGraphicsContextInstance, result);

		if (result != RC_OK)
		{
			return result;
		}

		if ((result = registerResourceType(mpResourceManagerInstance, pResourceLoader, pResourceFactory)) != RC_OK)
		{
			return result;
		}

		/// \note register font's resource type
		pResourceLoader = CreateFontLoader(mpResourceManagerInstance, mpFileSystemInstance, result);

		if (result != RC_OK)
		{
			return result;
		}

		pResourceFactory = CreateFontFactory(mpResourceManagerInstance, result);

		if (result != RC_OK)
		{
			return result;
		}

		if ((result = registerResourceType(mpResourceManagerInstance, pResourceLoader, pResourceFactory)) != RC_OK)
		{
			return result;
		}

		/// \note register static mesh's resource type
		pResourceLoader = CreateStaticMeshLoader(mpResourceManagerInstance, mpGraphicsContextInstance, mpFileSystemInstance, result);

		if (result != RC_OK)
		{
			return result;
		}

		pResourceFactory = CreateStaticMeshFactory(mpResourceManagerInstance, mpGraphicsContextInstance, result);

		if (result != RC_OK)
		{
			return result;
		}

		if ((result = registerResourceType(mpResourceManagerInstance, pResourceLoader, pResourceFactory)) != RC_OK)
		{
			return result;
		}

		return RC_OK;
	}

	TResult<TEngineSettings> CConfigFileEngineCoreBuilder::_readConfigurationFile(IFileSystem* pFileSystem, const std::string& configFilename)
	{
		TFileEntryId configFileHandle = pFileSystem->Open<IConfigFileReader>(configFilename, false).Get();

		IConfigFileReader* pConfigFileReader = pFileSystem->Get<IConfigFileReader>(configFileHandle);

		TEngineSettings settings;

		settings.mApplicationName       = pConfigFileReader->GetString("main", "name", "Default App");
		settings.mWindowWidth           = pConfigFileReader->GetInt("main", "width", 640);
		settings.mWindowHeight          = pConfigFileReader->GetInt("main", "height", 480);
		settings.mFlags                 = pConfigFileReader->GetInt("main", "flags", 0x0);
		settings.mMaxNumOfWorkerThreads = pConfigFileReader->GetInt("main", "max-num-worker-threads", std::thread::hardware_concurrency() - 1);

		settings.mTotalPreallocatedMemorySize = pConfigFileReader->GetInt("memory", "total-preallocated-memory-size", DefaultGlobalMemoryBlockSize);
		settings.mGraphicsContextType         = StringToGraphicsContextType(pConfigFileReader->GetString("graphics", "context-type", "unknown"));

		pConfigFileReader->Close();

		return TOkValue<TEngineSettings>(settings);
	}


	TDE2_API IEngineCoreBuilder* CreateConfigFileEngineCoreBuilder(TCreateEngineCoreCallback pEngineCoreFactoryCallback, const std::string& configFilename, E_RESULT_CODE& result)
	{
		CConfigFileEngineCoreBuilder* pEngineCoreBuilder = new (std::nothrow) CConfigFileEngineCoreBuilder();

		if (!pEngineCoreBuilder)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pEngineCoreBuilder->Init(pEngineCoreFactoryCallback, configFilename);

		if (result != RC_OK)
		{
			delete pEngineCoreBuilder;

			pEngineCoreBuilder = nullptr;
		}

		return dynamic_cast<IEngineCoreBuilder*>(pEngineCoreBuilder);
	}
}