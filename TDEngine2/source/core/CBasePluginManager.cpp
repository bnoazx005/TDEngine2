#include "./../../include/core/CBasePluginManager.h"
#include "./../../include/core/IPlugin.h"
#include "./../../include/core/IDLLManager.h"
#include "./../../include/core/IEngineCore.h"
#include "./../../include/core/IWindowSystem.h"
#include "./../../include/utils/CFileLogger.h"


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
		
		LOG_MESSAGE("[Plugin Manager] The plugin manager was initialized");

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

		E_RESULT_CODE result = UnloadAllPlugins();

		if (result != RC_OK)
		{
			return result;
		}

		delete this;

		LOG_MESSAGE("[Plugin Manager] The plugin manager was destroyed");

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

		LOG_MESSAGE("[Plugin Manager] The plugin manager tries to load a plugin (" + filename + ") ...");

		TCreatePluginCallback pCreatePluginCallback = (TCreatePluginCallback)mpDLLManager->GetSymbol(libraryHandler, TDE2_CREATE_PLUGIN_FUNC_NAME);

		if (!pCreatePluginCallback)
		{
			return RC_FAIL;
		}

		IPlugin* pLoadedPlugin = pCreatePluginCallback(mpEngineCore, result);

		if (result != RC_OK)
		{
			LOG_MESSAGE(std::string("[Plugin Manager] Can't load the plugin, error has happened (" + std::to_string(result) + ")"));

			return result;
		}

		mLoadedPlugins[filename] = pLoadedPlugin;

		LOG_MESSAGE("[Plugin Manager] The plugin was successfully loaded (" + filename + ")");

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

		LOG_MESSAGE("[Plugin Manager] The plugin was successfully unloaded (" + filename + ")");

		return RC_OK;
	}

	E_RESULT_CODE CBasePluginManager::UnloadAllPlugins()
	{
		E_RESULT_CODE result     = RC_OK;
		E_RESULT_CODE currResult = RC_OK;

		TPluginsMap::iterator currPluginIter = mLoadedPlugins.begin();

		while (currPluginIter != mLoadedPlugins.end())
		{
			currResult = UnloadPlugin((*currPluginIter).first);

			if (currResult != RC_OK)
			{
				result = currResult;
			}

			++currPluginIter;
		}

		return result;
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