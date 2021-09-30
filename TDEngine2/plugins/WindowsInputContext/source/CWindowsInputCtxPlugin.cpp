#include "./../include/CWindowsInputCtxPlugin.h"
#include <core/IEngineCore.h>
#include <core/IInputContext.h>
#include <core/IWindowSystem.h>
#include "./../include/CWindowsInputContext.h"


namespace TDEngine2
{
	CWindowsInputCtxPlugin::CWindowsInputCtxPlugin():
		CBaseObject()
	{
	}

	E_RESULT_CODE CWindowsInputCtxPlugin::Init(IEngineCore* pEngineCore)
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

		E_RESULT_CODE result = RC_OK;

		mpInputContext = CreateWindowsInputContext(pEngineCore->GetSubsystem<IWindowSystem>(), result);

		if (result != RC_OK)
		{
			return result;
		}

		if ((result = mpEngineCoreInstance->RegisterSubsystem(mpInputContext)) != RC_OK)
		{
			return result;
		}
		
		mIsInitialized = true;

		return RC_OK;
	}

	const TPluginInfo& CWindowsInputCtxPlugin::GetInfo() const
	{
		return mPluginInfo;
	}
}


extern "C" TDE2_API TDEngine2::IPlugin* TDE2_APIENTRY CreatePlugin(TDEngine2::IEngineCore* pEngineCore, TDEngine2::E_RESULT_CODE& result)
{
	TDEngine2::CWindowsInputCtxPlugin* pPluginInstance = new (std::nothrow) TDEngine2::CWindowsInputCtxPlugin();

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