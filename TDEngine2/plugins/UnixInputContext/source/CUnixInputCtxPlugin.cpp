#include "./../include/CUnixInputCtxPlugin.h"
#include <core/IEngineCore.h>
#include <core/IInputContext.h>
#include <core/IWindowSystem.h>
#include "./../include/CUnixInputContext.h"


namespace TDEngine2
{
	CUnixInputCtxPlugin::CUnixInputCtxPlugin():
		CBaseObject()
	{
	}

	E_RESULT_CODE CUnixInputCtxPlugin::Init(IEngineCore* pEngineCore)
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

		mpInputContext = CreateUnixInputContext(pEngineCore->GetSubsystem<IWindowSystem>(), result);

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

	E_RESULT_CODE CUnixInputCtxPlugin::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		delete this;

		mIsInitialized = false;

		return RC_OK;
	}

	const TPluginInfo& CUnixInputCtxPlugin::GetInfo() const
	{
		return mPluginInfo;
	}
}


extern "C" TDE2_API TDEngine2::IPlugin* TDE2_APIENTRY CreatePlugin(TDEngine2::IEngineCore* pEngineCore, TDEngine2::E_RESULT_CODE& result)
{
	TDEngine2::CUnixInputCtxPlugin* pPluginInstance = new (std::nothrow) TDEngine2::CUnixInputCtxPlugin();

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