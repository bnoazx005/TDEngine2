#include "../include/CD3D12GCtxPlugin.h"
#include "../include/CD3D12GraphicsContext.h"
#include "../include/CD3D12Resources.h"
#include "../include/CD3D12ShaderCompiler.h"
#include <core/IEngineCore.h>
#include <core/IGraphicsContext.h>
#include <core/IWindowSystem.h>
#include <core/IResourceManager.h>
#include <core/IFileSystem.h>
#include <core/IPluginManager.h>
#include <graphics/CBaseTexture2D.h>
#include <graphics/IGraphicsObjectManager.h>


namespace TDEngine2
{
	static const TPluginInfo PluginInfo 
	{
		"D3D12GraphicsContext",
		(TDE2_MAJOR_VERSION << 16) | TDE2_MINOR_VERSION,
		(TDE2_MAJOR_VERSION << 16) | TDE2_MINOR_VERSION,
	};


	CD3D12GCtxPlugin::CD3D12GCtxPlugin() :
		CBaseObject(), mpEngineCoreInstance(nullptr), mpGraphicsContext(nullptr)
	{
	}

	E_RESULT_CODE CD3D12GCtxPlugin::Init(IEngineCore* pEngineCore)
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

		mpGraphicsContext = TPtr<IGraphicsContext>(CreateD3D12GraphicsContext(pEngineCore->GetSubsystem<IWindowSystem>(), pEngineCore->GetSubsystem<IFileSystem>(), result));
		if (result != RC_OK)
		{
			return result;
		}

		TPtr<IShaderCompiler> pShaderCompilerInstance = TPtr<IShaderCompiler>(CreateD3D12ShaderCompiler(pEngineCore->GetSubsystem<IFileSystem>().Get(), result));
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

	const TPluginInfo& CD3D12GCtxPlugin::GetInfo() const
	{
		return PluginInfo;
	}
}


extern "C" TDE2_API TDEngine2::IPlugin* TDE2_APIENTRY CreatePlugin(TDEngine2::IEngineCore* pEngineCore, TDEngine2::E_RESULT_CODE& result)
{
	TDEngine2::CD3D12GCtxPlugin* pPluginInstance = new (std::nothrow) TDEngine2::CD3D12GCtxPlugin();

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