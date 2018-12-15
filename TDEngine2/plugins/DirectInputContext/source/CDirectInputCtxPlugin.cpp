#include "./../include/CDirectInputCtxPlugin.h"
#include <core/IEngineCore.h>
#include <core/IInputContext.h>
#include <core/IWindowSystem.h>
#include "./../include/CDirectInputContext.h"


namespace TDEngine2
{
	CDirectInputCtxPlugin::CDirectInputCtxPlugin():
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CDirectInputCtxPlugin::Init(IEngineCore* pEngineCore)
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

		mpInputContext = CreateDirectInputContext(dynamic_cast<IWindowSystem*>(pEngineCore->GetSubsystem(EST_WINDOW)), result);

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

	E_RESULT_CODE CDirectInputCtxPlugin::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		delete this;

		mIsInitialized = false;

		return RC_OK;
	}

	const TPluginInfo& CDirectInputCtxPlugin::GetInfo() const
	{
		return mPluginInfo;
	}
}


extern "C" TDE2_API TDEngine2::IPlugin* TDE2_APIENTRY CreatePlugin(TDEngine2::IEngineCore* pEngineCore, TDEngine2::E_RESULT_CODE& result)
{
	TDEngine2::CDirectInputCtxPlugin* pPluginInstance = new (std::nothrow) TDEngine2::CDirectInputCtxPlugin();

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