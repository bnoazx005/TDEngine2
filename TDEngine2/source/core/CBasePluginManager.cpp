#include "./../../include/core/CBasePluginManager.h"
#include "./../../include/core/IPlugin.h"
#include "./../../include/core/IDLLManager.h"
#include "./../../include/core/IEngineCore.h"
#include "./../../include/core/IWindowSystem.h"


namespace TDEngine2
{
	CBasePluginManager::CBasePluginManager():
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CBasePluginManager::Init(IEngineCore* pEngineCore)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pEngineCore)
		{
			return RC_INVALID_ARGS;
		}

		mpEngineCore = pEngineCore;

		IWindowSystem* pWindowSystem = dynamic_cast<IWindowSystem*>(pEngineCore->GetSubsystem(EST_WINDOW));

		if (!pWindowSystem)
		{
			return RC_FAIL;
		}

		mpDLLManager = pWindowSystem->GetDLLManagerInstance();

		if (!mpDLLManager)
		{
			return RC_FAIL;
		}
		
		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBasePluginManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CBasePluginManager::LoadPlugin(const std::string& filename)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (filename.empty())
		{
			return RC_INVALID_ARGS;
		}

		if (!mpDLLManager)
		{
			return RC_FAIL;
		}

		IPlugin* pPluginInstance = mLoadedPlugins[filename];

		if (pPluginInstance)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		TDynamicLibraryHandler libraryHandler = mpDLLManager->Load(filename, result);

		if (result != RC_OK)
		{
			return result;
		}

		TCreatePluginCallback pCreatePluginCallback = (TCreatePluginCallback)mpDLLManager->GetSymbol(libraryHandler, TDE2_CREATE_PLUGIN_FUNC_NAME);

		if (!pCreatePluginCallback)
		{
			return RC_FAIL;
		}

		IPlugin* pLoadedPlugin = pCreatePluginCallback(mpEngineCore, result);

		if (result != RC_OK)
		{
			return result;
		}

		mLoadedPlugins[filename] = pLoadedPlugin;

		return RC_OK;
	}

	E_RESULT_CODE CBasePluginManager::UnloadPlugin(const std::string& filename)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (filename.empty())
		{
			return RC_INVALID_ARGS;
		}

		if (!mpDLLManager)
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

	E_ENGINE_SUBSYSTEM_TYPE CBasePluginManager::GetType() const
	{
		return EST_PLUGIN_MANAGER;
	}


	TDE2_API IPluginManager* CreateBasePluginManager(IEngineCore* pEngineCore, E_RESULT_CODE& result)
	{
		CBasePluginManager* pPluginManagerInstance = new (std::nothrow) CBasePluginManager();

		if (!pPluginManagerInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pPluginManagerInstance->Init(pEngineCore);

		if (result != RC_OK)
		{
			delete pPluginManagerInstance;

			pPluginManagerInstance = nullptr;
		}

		return dynamic_cast<IPluginManager*>(pPluginManagerInstance);
	}
}