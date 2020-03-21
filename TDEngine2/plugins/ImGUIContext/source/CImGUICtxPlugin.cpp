#include "./../include/CImGUICtxPlugin.h"
#include <core/IEngineCore.h>
#include <core/IWindowSystem.h>
#include <core/IGraphicsContext.h>
#include <core/IInputContext.h>
#include <core/IImGUIContext.h>
#include <core/IResourceManager.h>
#include <graphics/IRenderer.h>
#include <graphics/IGraphicsObjectManager.h>


namespace TDEngine2
{
	CImGUICtxPlugin::CImGUICtxPlugin():
		CBaseObject()
	{
	}

	E_RESULT_CODE CImGUICtxPlugin::Init(IEngineCore* pEngineCore)
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

		mpImGUIContext = CreateImGUIContext(pEngineCore->GetSubsystem<IWindowSystem>(),
											pEngineCore->GetSubsystem<IRenderer>(),
											pEngineCore->GetSubsystem<IGraphicsContext>()->GetGraphicsObjectManager(),
											pEngineCore->GetSubsystem<IResourceManager>(),
											pEngineCore->GetSubsystem<IInputContext>(), result);

		if ((result != RC_OK) || ((result = mpEngineCoreInstance->RegisterSubsystem(mpImGUIContext)) != RC_OK))
		{
			return result;
		}
		
		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CImGUICtxPlugin::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		delete this;

		mIsInitialized = false;

		return RC_OK;
	}

	const TPluginInfo& CImGUICtxPlugin::GetInfo() const
	{
		return mPluginInfo;
	}
}


extern "C" TDE2_API TDEngine2::IPlugin* TDE2_APIENTRY CreatePlugin(TDEngine2::IEngineCore* pEngineCore, TDEngine2::E_RESULT_CODE& result)
{
	TDEngine2::CImGUICtxPlugin* pPluginInstance = new (std::nothrow) TDEngine2::CImGUICtxPlugin();

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