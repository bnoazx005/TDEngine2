#include "./../../include/core/CDefaultEngineCoreBuilder.h"
#include "./../../include/core/CEngineCore.h"
#include "./../../include/platform/win32/CWin32WindowSystem.h"
#include "./../../include/core/IPlugin.h"
#include "./../../include/core/IFileSystem.h"
#include "./../../include/platform/win32/CWin32FileSystem.h"
#include "./../../include/platform/unix/CUnixWindowSystem.h"
#include "./../../include/core/CResourceManager.h"
#include "./../../include/core/CBaseJobManager.h"
#include "./../../include/core/IJobManager.h"
#include "./../../include/core/CBasePluginManager.h"
#include "./../../include/platform/unix/CUnixFileSystem.h"
#include "./../../include/platform/CTextFileReader.h"
#include "./../../include/platform/CConfigFileReader.h"
#include "./../../include/core/CEventManager.h"
#include "./../../include/core/memory/CMemoryManager.h"
#include "./../../include/core/memory/CLinearAllocator.h"
#include "./../../include/core/memory/CPoolAllocator.h"
#include "./../../include/core/memory/CStackAllocator.h"
#include <memory>


namespace TDEngine2
{
	CDefaultEngineCoreBuilder::CDefaultEngineCoreBuilder():
		mIsInitialized(false), mpEngineCoreInstance(nullptr), mpWindowSystemInstance(nullptr), mpJobManagerInstance(nullptr),
		mpPluginManagerInstance(nullptr)
	{
	}

	/*!
		\brief The method initialized the builder's object

		\param[in] A callback to a factory's function of IEngineCore's objects

		\return RC_OK if everything went ok, or some other code, which describes an error
	*/

	E_RESULT_CODE CDefaultEngineCoreBuilder::Init(TCreateEngineCoreCallback pEngineCoreFactoryCallback)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pEngineCoreFactoryCallback)
		{
			return RC_INVALID_ARGS;
		}

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

	E_RESULT_CODE CDefaultEngineCoreBuilder::ConfigureGraphicsContext(E_GRAPHICS_CONTEXT_GAPI_TYPE type)
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

		return RC_OK;
	}

	E_RESULT_CODE CDefaultEngineCoreBuilder::ConfigureWindowSystem(const std::string& name, U32 width, U32 height, U32 flags)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mpWindowSystemInstance = nullptr;

		E_RESULT_CODE result = RC_OK;

#if defined (TDE2_USE_WIN32PLATFORM)																/// Win32 Platform
		mpWindowSystemInstance = CreateWin32WindowSystem(name, width, height, flags, result);
#elif defined (TDE2_USE_UNIXPLATFORM)
		mpWindowSystemInstance = CreateUnixWindowSystem(name, width, height, flags, result);
#else
#endif

		if (result != RC_OK)
		{
			return result;
		}

		return mpEngineCoreInstance->RegisterSubsystem(mpWindowSystemInstance);
	}

	E_RESULT_CODE CDefaultEngineCoreBuilder::ConfigureFileSystem()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		IFileSystem* pFileSystem = nullptr;
		
#if defined (TDE2_USE_WIN32PLATFORM)
		pFileSystem = CreateWin32FileSystem(result);
#elif defined (TDE2_USE_UNIXPLATFORM)
		pFileSystem = CreateUnixFileSystem(result);
#else
#endif
		
		/// register known file types factories
		pFileSystem->RegisterFileFactory<CTextFileReader>(CreateTextFileReader);
		pFileSystem->RegisterFileFactory<CConfigFileReader>(CreateConfigFileReader);

		if (result != RC_OK)
		{
			return result;
		}
		
		return mpEngineCoreInstance->RegisterSubsystem(dynamic_cast<IEngineSubsystem*>(pFileSystem));
	}

	E_RESULT_CODE CDefaultEngineCoreBuilder::ConfigureResourceManager()
	{
		if (!mIsInitialized || !mpJobManagerInstance)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		IEngineSubsystem* pResourceManager = CreateResourceManager(mpJobManagerInstance, result);

		if (result != RC_OK)
		{
			return result;
		}
		
		return mpEngineCoreInstance->RegisterSubsystem(pResourceManager);
	}

	E_RESULT_CODE CDefaultEngineCoreBuilder::ConfigureJobManager(U32 maxNumOfThreads)
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

	E_RESULT_CODE CDefaultEngineCoreBuilder::ConfigurePluginManager()
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

	E_RESULT_CODE CDefaultEngineCoreBuilder::ConfigureEventManager()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		IEngineSubsystem* pEventManager = CreateEventManager(result);

		if (result != RC_OK)
		{
			return result;
		}
		
		return mpEngineCoreInstance->RegisterSubsystem(pEventManager);
	}
	
	E_RESULT_CODE CDefaultEngineCoreBuilder::ConfigureMemoryManager(U32 totalMemorySize)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		IMemoryManager* pMemoryManager = CreateMemoryManager(totalMemorySize, result);

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

			if ((result = pMemoryManager->RegisterFactory(pAllocatorFactory)) != RC_OK)
			{
				return result;
			}
		}

		return mpEngineCoreInstance->RegisterSubsystem(pMemoryManager);
	}

	IEngineCore* CDefaultEngineCoreBuilder::GetEngineCore()
	{
		return mpEngineCoreInstance;
	}

	
	TDE2_API IEngineCoreBuilder* CreateDefaultEngineCoreBuilder(TCreateEngineCoreCallback pEngineCoreFactoryCallback, E_RESULT_CODE& result)
	{
		CDefaultEngineCoreBuilder* pEngineCoreBuilder = new (std::nothrow) CDefaultEngineCoreBuilder();

		if (!pEngineCoreBuilder)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pEngineCoreBuilder->Init(pEngineCoreFactoryCallback);

		if (result != RC_OK)
		{
			delete pEngineCoreBuilder;

			pEngineCoreBuilder = nullptr;
		}

		return dynamic_cast<IEngineCoreBuilder*>(pEngineCoreBuilder);
	}
}