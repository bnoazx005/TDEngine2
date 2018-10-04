#include "./../include/COGLGCtxPlugin.h"
#include <core/IEngineCore.h>
#include <core/IGraphicsContext.h>


namespace TDEngine2
{
	COGLGCtxPlugin::COGLGCtxPlugin() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE COGLGCtxPlugin::Init(IEngineCore* pEngineCore)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pEngineCore)
		{
			return RC_INVALID_ARGS;
		}

		/// \todo Implement plugin's registration

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE COGLGCtxPlugin::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		if ((result = mpGraphicsContext->Free()) != RC_OK)
		{
			return result;
		}

		delete this;

		mIsInitialized = false;

		return RC_OK;
	}

	const TPluginInfo& COGLGCtxPlugin::GetInfo() const
	{
		return mPluginInfo;
	}
}


extern "C" TDE2_API TDEngine2::IPlugin* CreatePlugin(TDEngine2::IEngineCore* pEngineCore, TDEngine2::E_RESULT_CODE& result)
{
	TDEngine2::COGLGCtxPlugin* pPluginInstance = new (std::nothrow) TDEngine2::COGLGCtxPlugin();

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