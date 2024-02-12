#include "../include/CTDE2VideoPlayersPlugin.h"
#include <core/IEngineCore.h>
#if TDE2_EDITORS_ENABLED
#include <editor/IEditorsManager.h>
#endif
#include <ecs/IWorld.h>
#include <scene/ISceneManager.h>


namespace TDEngine2
{
	CVideoPlayersPlugin::CVideoPlayersPlugin():
		CBaseObject()
	{
	}

	E_RESULT_CODE CVideoPlayersPlugin::Init(IEngineCore* pEngineCore)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pEngineCore)
		{
			return RC_INVALID_ARGS;
		}

		mpEngineCoreInstance = pEngineCore;
		
		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CVideoPlayersPlugin::OnRegisterSystems(IEngineCore* pEngineCore, IWorld* pWorld)
	{
		E_RESULT_CODE result = RC_OK;

		return result;
	}

	E_RESULT_CODE CVideoPlayersPlugin::OnRegisterComponents(IEngineCore* pEngineCore, IWorld* pWorld)
	{
		return RC_OK;
	}

	const TPluginInfo& CVideoPlayersPlugin::GetInfo() const
	{
		static const TPluginInfo pluginInfo
		{
			"TDE2VideoPlayers",
			(0 << 16) | 7,
			(0 << 16) | 7,
		};

		return pluginInfo;
	}
}


extern "C" TDE2_API TDEngine2::IPlugin* TDE2_APIENTRY CreatePlugin(TDEngine2::IEngineCore* pEngineCore, TDEngine2::E_RESULT_CODE& result)
{
	TDEngine2::CVideoPlayersPlugin* pPluginInstance = new (std::nothrow) TDEngine2::CVideoPlayersPlugin();

	if (!pPluginInstance)
	{
		result = TDEngine2::RC_OUT_OF_MEMORY;

		return nullptr;
	}

	result = pPluginInstance->Init(pEngineCore);

	if (result != TDEngine2::RC_OK)
	{
		delete pPluginInstance;

		pPluginInstance = nullptr;
	}

	return dynamic_cast<TDEngine2::IPlugin*>(pPluginInstance);
}