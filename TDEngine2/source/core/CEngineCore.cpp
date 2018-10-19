#include "./../../include/core/CEngineCore.h"
#include "./../../include/core/IEngineSubsystem.h"
#include "./../../include/core/IWindowSystem.h"
#include "./../../include/core/IEngineListener.h"
#include "./../../include/utils/CFileLogger.h"
#include "./../../include/core/IPlugin.h"
#include "./../../include/core/IDLLManager.h"
#include <cstring>
#include <algorithm>


namespace TDEngine2
{
	CEngineCore::CEngineCore():
		mIsInitialized(false)
	{
	}

	CEngineCore::~CEngineCore()
	{
	}

	E_RESULT_CODE CEngineCore::Init()
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		memset(&mSubsystems, 0, sizeof(mSubsystems));

		mpDLLManager = nullptr;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (_onNotifyEngineListeners(EET_ONFREE) != RC_OK)
		{
			return RC_FAIL;
		}

#if defined (_DEBUG)
		if (MainLogger->Free() != RC_OK)
		{
			return RC_FAIL;
		}
#endif

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::Run()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (_onNotifyEngineListeners(EET_ONSTART) != RC_OK)
		{
			return RC_FAIL;
		}

		IWindowSystem* pWindowSystem = dynamic_cast<IWindowSystem*>(mSubsystems[EST_WINDOW]);

		if (!pWindowSystem)
		{
			return RC_FAIL;
		}

		/// \todo replace _onFrameUpdateCallback with a user defined callback
		pWindowSystem->Run(std::bind(&CEngineCore::_onFrameUpdateCallback, this));
		
		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::Quit()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		IWindowSystem* pWindowSystem = dynamic_cast<IWindowSystem*>(mSubsystems[EST_WINDOW]);

		if (!pWindowSystem)
		{
			return RC_FAIL;
		}

		if ((result = pWindowSystem->Quit()) != RC_OK)
		{
			return result;
		}

		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::RegisterSubsystem(IEngineSubsystem* pSubsystem)
	{
		E_ENGINE_SUBSYSTEM_TYPE subsystemType = EST_UNKNOWN;

		if (!pSubsystem || ((subsystemType = pSubsystem->GetType()) == EST_UNKNOWN))
		{
			return RC_FAIL;
		}

		if (mSubsystems[subsystemType])
		{
			/// \todo check up is choosen slot free, if not print warning and try to free the memory it stores
		}

		mSubsystems[subsystemType] = pSubsystem;

		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::UnregisterSubsystem(E_ENGINE_SUBSYSTEM_TYPE subsystemType)
	{
		/// \todo Implement UnregisterSubsystem method

		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CEngineCore::RegisterListener(IEngineListener* pListener)
	{
		if (!pListener)
		{
			return RC_INVALID_ARGS;
		}

		/// prevent duplicating instances 
		TListenersArray::const_iterator copyIter = std::find(mEngineListeners.cbegin(), mEngineListeners.cend(), pListener);

		if (copyIter != mEngineListeners.cend())
		{
			return RC_FAIL;
		}

		pListener->SetEngineInstance(this); /// inject engine's instance into the listener

		mEngineListeners.push_back(pListener);

		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::UnregisterListener(IEngineListener* pListener)
	{
		if (!pListener)
		{
			return RC_INVALID_ARGS;
		}
		
		TListenersArray::iterator entityIter = std::find(mEngineListeners.begin(), mEngineListeners.end(), pListener);

		if (entityIter == mEngineListeners.cend())
		{
			return RC_FAIL;
		}

		mEngineListeners.erase(entityIter);

		return RC_OK;
	}


	E_RESULT_CODE CEngineCore::LoadPlugin(const std::string& filename)
	{
		if (filename.empty())
		{
			return RC_INVALID_ARGS;
		}

		/// \todo check up filename's ending 

		IDLLManager* pDllManager = _getDLLManagerInstance(dynamic_cast<const IWindowSystem*>(mSubsystems[EST_WINDOW]));

		if (!pDllManager)
		{
			return RC_FAIL;
		}

		IPlugin* pPluginInstance = mLoadedPlugins[filename];

		if (pPluginInstance)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		TDynamicLibraryHandler libraryHandler = pDllManager->Load(filename, result);

		if (result != RC_OK)
		{
			return result;
		}

		TCreatePluginCallback pCreatePluginCallback = (TCreatePluginCallback)pDllManager->GetSymbol(libraryHandler, TDE2_CREATE_PLUGIN_FUNC_NAME);
		
		if (!pCreatePluginCallback)
		{
			return RC_FAIL;
		}

		IPlugin* pLoadedPlugin = pCreatePluginCallback(this, result);

		if (result != RC_OK)
		{
			return result;
		}

		mLoadedPlugins[filename] = pLoadedPlugin;
		
		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::UnloadPlugin(const std::string& filename)
	{
		if (filename.empty())
		{
			return RC_INVALID_ARGS;
		}

		/// \todo check up filename's ending 

		IDLLManager* pDllManager = _getDLLManagerInstance(dynamic_cast<const IWindowSystem*>(mSubsystems[EST_WINDOW]));

		if (!pDllManager)
		{
			return RC_FAIL;
		}

		IPlugin* pPluginInstance = mLoadedPlugins[filename];

		if (!pPluginInstance)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		if ((result = pPluginInstance->Free()) != RC_OK)
		{
			return result;
		}

		mLoadedPlugins[filename] = nullptr;

		return RC_OK;
	}
	
	IEngineSubsystem* CEngineCore::GetSubsystem(E_ENGINE_SUBSYSTEM_TYPE type) const
	{
		if (type == EST_UNKNOWN)
		{
			return nullptr;
		}

		return mSubsystems[type];
	}

	ILogger* CEngineCore::GetLogger() const
	{
#if defined (_DEBUG)
		return MainLogger;
#else
		return nullptr;
#endif
	}

	ITimer* CEngineCore::GetTimer() const
	{
		IWindowSystem* pWindowSystem = dynamic_cast<IWindowSystem*>(mSubsystems[EST_WINDOW]);
		
		if (!pWindowSystem)
		{
			return nullptr;
		}

		return pWindowSystem->GetTimer();
	}

	void CEngineCore::_onFrameUpdateCallback()
	{
		_onNotifyEngineListeners(EET_ONUPDATE);
	}

	E_RESULT_CODE CEngineCore::_onNotifyEngineListeners(E_ENGINE_EVENT_TYPE eventType)
	{
		if (mEngineListeners.empty())
		{
			return RC_OK;
		}

		E_RESULT_CODE resultCode = RC_OK;

		for (IEngineListener* pListener : mEngineListeners)
		{
			if (!pListener)
			{
				continue;
			}

			switch (eventType)
			{
				case EET_ONSTART:
					resultCode = pListener->OnStart();
					break;
				case EET_ONUPDATE:
					resultCode = pListener->OnUpdate(0.0f /*! \todo replace with delta time's value */);
					break;
				case EET_ONFREE:
					resultCode = pListener->OnFree();
					break;
			}

			if (resultCode != RC_OK)
			{
				return resultCode;
			}
		}

		return RC_OK;
	}

	IDLLManager* CEngineCore::_getDLLManagerInstance(const IWindowSystem* pWindowSystem)
	{
		if (!pWindowSystem && !mpDLLManager)
		{
			return nullptr;
		}

		if (!mpDLLManager)
		{
			mpDLLManager = pWindowSystem->GetDLLManagerInstance();
		}

		return mpDLLManager;
	}
	

	IEngineCore* CreateEngineCore(E_RESULT_CODE& result)
	{
		CEngineCore* pEngineCore = new (std::nothrow) CEngineCore();

		if (!pEngineCore)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pEngineCore->Init();

		if (result != RC_OK)
		{
			delete pEngineCore;

			pEngineCore = nullptr;
		}

		return dynamic_cast<IEngineCore*>(pEngineCore);
	}
}