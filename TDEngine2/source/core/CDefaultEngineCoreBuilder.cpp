#include "./../../include/core/CDefaultEngineCoreBuilder.h"
#include "./../../include/core/CEngineCore.h"
#include "./../../include/core/IPlugin.h"
#include "./../../include/core/IFileSystem.h"
#include "./../../include/core/CResourceManager.h"
#include "./../../include/core/CBaseJobManager.h"
#include "./../../include/core/IJobManager.h"
#include "./../../include/core/CBasePluginManager.h"
#include "./../../include/core/CEventManager.h"
#include "./../../include/core/IInputContext.h"
#include "./../../include/core/IGraphicsContext.h"
#include "./../../include/core/CFont.h"
#include "./../../include/core/memory/CMemoryManager.h"
#include "./../../include/core/memory/CLinearAllocator.h"
#include "./../../include/core/memory/CPoolAllocator.h"
#include "./../../include/core/memory/CStackAllocator.h"
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
#include "./../../include/editor/CProfilerEditorWindow.h"
#include "./../../include/editor/CPerfProfiler.h"
#include "./../../include/editor/CLevelEditorWindow.h"
#include "./../../include/editor/CDevConsoleWindow.h"
#include "./../../include/editor/CSelectionManager.h"
#include "./../../include/graphics/CFramePostProcessor.h"
#include "./../../include/graphics/CBasePostProcessingProfile.h"
#include "./../../include/graphics/IDebugUtility.h"
#include "./../../include/graphics/IGraphicsObjectManager.h"
#include "./../../include/graphics/animation/CAnimationClip.h"
#include <memory>
#include <cstring>
#include <tuple>


namespace TDEngine2
{
	CDefaultEngineCoreBuilder::CDefaultEngineCoreBuilder():
		mIsInitialized(false), mpEngineCoreInstance(nullptr), mpWindowSystemInstance(nullptr), mpJobManagerInstance(nullptr),
		mpPluginManagerInstance(nullptr), mpGraphicsContextInstance(nullptr), mpResourceManagerInstance(nullptr)
	{
	}

	/*!
		\brief The method initialized the builder's object

		\param[in] A callback to a factory's function of IEngineCore's objects

		\return RC_OK if everything went ok, or some other code, which describes an error
	*/

	E_RESULT_CODE CDefaultEngineCoreBuilder::Init(TCreateEngineCoreCallback pEngineCoreFactoryCallback, const TEngineSettings& settings)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pEngineCoreFactoryCallback)
		{
			return RC_INVALID_ARGS;
		}

		memcpy(&mEngineSettings, &settings, sizeof(settings));

		E_RESULT_CODE result = RC_OK;

		mpEngineCoreInstance = pEngineCoreFactoryCallback(result);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CDefaultEngineCoreBuilder::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CDefaultEngineCoreBuilder::_configureGraphicsContext(E_GRAPHICS_CONTEXT_GAPI_TYPE type)
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

	E_RESULT_CODE CDefaultEngineCoreBuilder::_configureWindowSystem(const std::string& name, U32 width, U32 height, U32 flags)
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

	E_RESULT_CODE CDefaultEngineCoreBuilder::_configureFileSystem()
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
		if (((result = mpFileSystemInstance->RegisterFileFactory<ITextFileReader>({ CreateTextFileReader, E_FILE_FACTORY_TYPE::READER })) != RC_OK) ||
			((result = mpFileSystemInstance->RegisterFileFactory<IConfigFileReader>({ CreateConfigFileReader, E_FILE_FACTORY_TYPE::READER })) != RC_OK) ||
			((result = mpFileSystemInstance->RegisterFileFactory<IBinaryFileReader>({ CreateBinaryFileReader, E_FILE_FACTORY_TYPE::READER })) != RC_OK) ||
			((result = mpFileSystemInstance->RegisterFileFactory<IBinaryFileWriter>({ CreateBinaryFileWriter, E_FILE_FACTORY_TYPE::WRITER })) != RC_OK) ||
			((result = mpFileSystemInstance->RegisterFileFactory<IImageFileWriter>({ CreateImageFileWriter, E_FILE_FACTORY_TYPE::WRITER })) != RC_OK) ||
			((result = mpFileSystemInstance->RegisterFileFactory<IYAMLFileReader>({ CreateYAMLFileReader, E_FILE_FACTORY_TYPE::READER })) != RC_OK) ||
			((result = mpFileSystemInstance->RegisterFileFactory<IYAMLFileWriter>({ CreateYAMLFileWriter, E_FILE_FACTORY_TYPE::WRITER })) != RC_OK) ||
			((result = mpFileSystemInstance->RegisterFileFactory<IBinaryMeshFileReader>({ CreateBinaryMeshFileReader, E_FILE_FACTORY_TYPE::READER })) != RC_OK))
		{
			return result;
		}
		
		if ((result = mpFileSystemInstance->Mount("./../../Resources/Shaders/", "Shaders")) != RC_OK)
		{
			return result;
		}

		return mpEngineCoreInstance->RegisterSubsystem(dynamic_cast<IEngineSubsystem*>(mpFileSystemInstance));
	}

	E_RESULT_CODE CDefaultEngineCoreBuilder::_configureResourceManager()
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

	E_RESULT_CODE CDefaultEngineCoreBuilder::_configureJobManager(U32 maxNumOfThreads)
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

	E_RESULT_CODE CDefaultEngineCoreBuilder::_configurePluginManager()
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

	E_RESULT_CODE CDefaultEngineCoreBuilder::_configureEventManager()
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

	E_RESULT_CODE CDefaultEngineCoreBuilder::_configureRenderer()
	{
		if (!mIsInitialized || !mpMemoryManagerInstance || !mpGraphicsContextInstance || !mpResourceManagerInstance)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;
		
		IAllocator* pAllocator = mpMemoryManagerInstance->CreateAllocator<CStackAllocator>({ static_cast<U32>(NumOfRenderQueuesGroup + 1) * PerRenderQueueMemoryBlockSize }, "Renderer");

		IRenderer* pRenderer = CreateForwardRenderer(mpGraphicsContextInstance, mpResourceManagerInstance, pAllocator, nullptr, result);
		if (result != RC_OK)
		{
			return result;
		}

		IFramePostProcessor* pFramePostProcessor = CreateFramePostProcessor(pRenderer, mpGraphicsContextInstance->GetGraphicsObjectManager(), mpWindowSystemInstance, result);
		if (result != RC_OK)
		{
			return result;
		}

		if ((result = pRenderer->SetFramePostProcessor(pFramePostProcessor)) != RC_OK)
		{
			return result;
		}

		return mpEngineCoreInstance->RegisterSubsystem(dynamic_cast<IEngineSubsystem*>(pRenderer));
	}
	
	E_RESULT_CODE CDefaultEngineCoreBuilder::_configureMemoryManager(U32 totalMemorySize)
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

	E_RESULT_CODE CDefaultEngineCoreBuilder::_configureInputContext()
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

	E_RESULT_CODE CDefaultEngineCoreBuilder::_configureImGUIContext()
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

	E_RESULT_CODE CDefaultEngineCoreBuilder::_configureEditorsManager()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

#if TDE2_EDITORS_ENABLED
		E_RESULT_CODE result = RC_OK;

		IInputContext* pInputContext = mpEngineCoreInstance->GetSubsystem<IInputContext>();

		IEditorsManager* pEditorsManager = CreateEditorsManager(pInputContext,
																mpEngineCoreInstance->GetSubsystem<IImGUIContext>(),
																mpEventManagerInstance,
																mpEngineCoreInstance->GetWorldInstance(),
																result);

		static const U32 mainThreadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
		
		auto pDebugUtility = mpGraphicsContextInstance->GetGraphicsObjectManager()->CreateDebugUtility(mpResourceManagerInstance, mpEngineCoreInstance->GetSubsystem<IRenderer>()).Get();

		std::tuple<std::string, IEditorWindow*> builtinEditors[]
		{
			{ CEditorsManager::mEditorNamesMap.at(E_EDITOR_TYPE::PROFILER), CreateProfilerEditorWindow(CPerfProfiler::Get(), result) },
			{ CEditorsManager::mEditorNamesMap.at(E_EDITOR_TYPE::LEVEL_EDITOR), CreateLevelEditorWindow(pEditorsManager, pInputContext, pDebugUtility, result) },
			{ CEditorsManager::mEditorNamesMap.at(E_EDITOR_TYPE::DEV_CONSOLE), CreateDevConsoleWindow(result) },
		};

		dynamic_cast<CProfilerEditorWindow*>(std::get<1>(builtinEditors[0]))->SetMainThreadID(mainThreadID);

		for (auto& currEditorEntry : builtinEditors)
		{
			result = result | pEditorsManager->RegisterEditor(std::get<std::string>(currEditorEntry), std::get<IEditorWindow*>(currEditorEntry));
		}

		if (result != RC_OK)
		{
			return result;
		}

		if (ISelectionManager* pSelectionManager = CreateSelectionManager(mpResourceManagerInstance, mpWindowSystemInstance, mpGraphicsContextInstance, pEditorsManager, result))
		{
			result = result | pEditorsManager->SetSelectionManager(pSelectionManager);
			result = result | mpEngineCoreInstance->GetSubsystem<IRenderer>()->SetSelectionManager(pSelectionManager);

			if (result != RC_OK)
			{
				return result;
			}
		}

		if (result != RC_OK)
		{
			return result;
		}

		return mpEngineCoreInstance->RegisterSubsystem(pEditorsManager);
#else
		return RC_OK;
#endif
	}

	IEngineCore* CDefaultEngineCoreBuilder::GetEngineCore()
	{
		PANIC_ON_FAILURE(_configureMemoryManager(mEngineSettings.mTotalPreallocatedMemorySize));
		PANIC_ON_FAILURE(_configureJobManager(mEngineSettings.mMaxNumOfWorkerThreads));
		PANIC_ON_FAILURE(_configureFileSystem());
		PANIC_ON_FAILURE(_configureEventManager());
		PANIC_ON_FAILURE(_configureResourceManager());
		PANIC_ON_FAILURE(_configureWindowSystem(mEngineSettings.mApplicationName, mEngineSettings.mWindowWidth, mEngineSettings.mWindowHeight, mEngineSettings.mFlags));
		PANIC_ON_FAILURE(_configurePluginManager());
		PANIC_ON_FAILURE(_configureGraphicsContext(mEngineSettings.mGraphicsContextType));
		PANIC_ON_FAILURE(_configureInputContext());

		mpFileSystemInstance->SetJobManager(mpJobManagerInstance);
		LOG_MESSAGE(std::string("[Default Engine Core Builder] Async file I/O operations status: ").append(mpFileSystemInstance->IsStreamingEnabled() ? "enabled" : "disabled"));

		E_RESULT_CODE result = _registerBuiltinInfrastructure();

		if (result != RC_OK)
		{
			LOG_WARNING("[Default Engine Core Builder] Couldn't register built-in types for the resource manager");

			return mpEngineCoreInstance;
		}

		PANIC_ON_FAILURE(_configureRenderer());
		PANIC_ON_FAILURE(_configureImGUIContext());
		PANIC_ON_FAILURE(_configureEditorsManager());

		return mpEngineCoreInstance;
	}

	E_RESULT_CODE CDefaultEngineCoreBuilder::_registerBuiltinInfrastructure()
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

		std::tuple<ResourceLoaderFactoryFunctor, ResourceFactoryFactoryFunctor> builtinResourcesConstructorsTable[]
		{
			{ CreateBaseMaterialLoader, CreateBaseMaterialFactory },
			{ CreateTextureAtlasLoader, CreateTextureAtlasFactory },
			{ CreateStaticMeshLoader, CreateStaticMeshFactory },
			{ CreateBasePostProcessingProfileLoader, CreateBasePostProcessingProfileFactory },
			{ CreateAnimationClipLoader, CreateAnimationClipFactory },
		};

		/// create material loader
		E_RESULT_CODE result = RC_OK;

		IResourceLoader* pResourceLoader = nullptr;
		IResourceFactory* pResourceFactory = nullptr;

		for (auto&& currResourceConstructors : builtinResourcesConstructorsTable)
		{
			pResourceLoader = std::get<ResourceLoaderFactoryFunctor>(currResourceConstructors)(mpResourceManagerInstance, mpGraphicsContextInstance, mpFileSystemInstance, result);

			if (result != RC_OK)
			{
				return result;
			}

			/// create a factory
			pResourceFactory = std::get<ResourceFactoryFactoryFunctor>(currResourceConstructors)(mpResourceManagerInstance, mpGraphicsContextInstance, result);

			if (result != RC_OK)
			{
				return result;
			}

			/// \note register a resource type
			if ((result = registerResourceType(mpResourceManagerInstance, pResourceLoader, pResourceFactory)) != RC_OK)
			{
				return result;
			}
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

		return RC_OK;
	}

	
	TDE2_API IEngineCoreBuilder* CreateDefaultEngineCoreBuilder(TCreateEngineCoreCallback pEngineCoreFactoryCallback, const TEngineSettings& settings, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEngineCoreBuilder, CDefaultEngineCoreBuilder, result, pEngineCoreFactoryCallback, settings);
	}
}