#include "../../include/core/CBasePluginManager.h"
#include "../../include/core/IPlugin.h"
#include "../../include/core/IDLLManager.h"
#include "../../include/core/IEngineCore.h"
#include "../../include/core/IWindowSystem.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/ecs/IWorld.h"
#if defined (TDE2_USE_WINPLATFORM)		
#include "../../include/platform/win32/CWin32DLLManager.h"
#elif defined (TDE2_USE_UNIXPLATFORM)
#include "../../include/platform/unix/CUnixDLLManager.h"
#else
#endif


namespace TDEngine2
{
	CBasePluginManager::CBasePluginManager():
		CBaseObject()
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

		E_RESULT_CODE result = RC_OK;

#if defined (TDE2_USE_WINPLATFORM)																/// Win32 Platform
		mpDLLManager = TPtr<IDLLManager>(CreateWin32DLLManager(result));
#elif defined (TDE2_USE_UNIXPLATFORM)
		mpDLLManager = TPtr<IDLLManager>(CreateUnixDLLManager(result));
#else
#endif

		if (!mpDLLManager)
		{
			return RC_FAIL;
		}
		
		LOG_MESSAGE("[Plugin Manager] The plugin manager was initialized");

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBasePluginManager::_onFreeInternal()
	{
		E_RESULT_CODE result = UnloadAllPlugins();

		if (result != RC_OK)
		{
			return result;
		}

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

		auto pPluginInstance = mLoadedPlugins[filename];

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

		auto pPluginInstance = mLoadedPlugins[filename];

		if (!pPluginInstance)
		{
			return RC_FAIL;
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

		mLoadedPlugins.clear();

		return result;
	}

	E_RESULT_CODE CBasePluginManager::RegisterECSComponents(TPtr<IWorld> pWorld)
	{
		if (!pWorld)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		for (auto&& currPlugin : mLoadedPlugins)
		{
			if (auto pECSPlugin = dynamic_cast<IECSPlugin*>(currPlugin.second.Get()))
			{
				result = result | pECSPlugin->OnRegisterComponents(mpEngineCore, pWorld.Get());
			}
		}

		return result;
	}

	E_RESULT_CODE CBasePluginManager::RegisterECSSystems(TPtr<IWorld> pWorld)
	{
		if (!pWorld)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		for (auto&& currPlugin : mLoadedPlugins)
		{
			if (auto pECSPlugin = dynamic_cast<IECSPlugin*>(currPlugin.second.Get()))
			{
				result = result | pECSPlugin->OnRegisterSystems(mpEngineCore, pWorld.Get());
			}
		}

		return result;
	}

	E_ENGINE_SUBSYSTEM_TYPE CBasePluginManager::GetType() const
	{
		return EST_PLUGIN_MANAGER;
	}


	TDE2_API IPluginManager* CreateBasePluginManager(IEngineCore* pEngineCore, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IPluginManager, CBasePluginManager, result, pEngineCore);
	}
}