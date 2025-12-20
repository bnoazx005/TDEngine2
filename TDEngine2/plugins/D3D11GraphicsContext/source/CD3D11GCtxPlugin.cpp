#include "../include/CD3D11GCtxPlugin.h"
#include "../include/CD3D11GraphicsContext.h"
#include "../include/CD3D11Resources.h"
#include <core/IEngineCore.h>
#include <core/IGraphicsContext.h>
#include <core/IWindowSystem.h>
#include <core/IResourceManager.h>
#include <core/IFileSystem.h>
#include <graphics/CBaseTexture2D.h>
#include <graphics/IGraphicsObjectManager.h>


namespace TDEngine2
{
	static const TPluginInfo PluginInfo 
	{
		"D3D11GraphicsContext",
		(0 << 16) | 5,
		(0 << 16) | 5,
	};


	CD3D11GCtxPlugin::CD3D11GCtxPlugin() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CD3D11GCtxPlugin::Init(IEngineCore* pEngineCore)
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

		mpGraphicsContext = TPtr<IGraphicsContext>(CreateD3D11GraphicsContext(pEngineCore->GetSubsystem<IWindowSystem>(), pEngineCore->GetSubsystem<IFileSystem>(), result));

		if (result != RC_OK)
		{
			return result;
		}

		TPtr<IShaderCompiler> pShaderCompilerInstance = TPtr<IShaderCompiler>(CreateD3D11ShaderCompiler(pEngineCore->GetSubsystem<IFileSystem>().Get(), result));
		if (result != RC_OK)
		{
			return result;
		}

		if ((result = mpGraphicsContext->GetGraphicsObjectManager()->SetShaderCompiler(pShaderCompilerInstance)) != RC_OK)
		{
			return result;
		}
		
		if ((result = pEngineCore->RegisterSubsystem(DynamicPtrCast<IEngineSubsystem>(mpGraphicsContext))) != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	const TPluginInfo& CD3D11GCtxPlugin::GetInfo() const
	{
		return PluginInfo;
	}
}


extern "C" TDE2_API TDEngine2::IPlugin* TDE2_APIENTRY CreatePlugin(TDEngine2::IEngineCore* pEngineCore, TDEngine2::E_RESULT_CODE& result)
{
	TDEngine2::CD3D11GCtxPlugin* pPluginInstance = new (std::nothrow) TDEngine2::CD3D11GCtxPlugin();

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