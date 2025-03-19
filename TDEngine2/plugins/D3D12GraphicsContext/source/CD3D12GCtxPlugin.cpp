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
#include <graphics/CBaseShaderLoader.h>
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


	static E_RESULT_CODE RegisterFactories(IEngineCore* pEngineCore)
	{
		IResourceManager* pResourceManager = pEngineCore->GetSubsystem<IResourceManager>().Get();
		IGraphicsContext* pGraphicsContext = pEngineCore->GetSubsystem<IGraphicsContext>().Get();

		if (!pResourceManager || !pGraphicsContext)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		auto factoryFunctions =
		{
			CreateD3D12ShaderFactory,
		};

		IResourceFactory* pFactoryInstance = nullptr;

		for (auto currFactoryCallback : factoryFunctions)
		{
			pFactoryInstance = currFactoryCallback(pResourceManager, pGraphicsContext, result);

			if (result != RC_OK)
			{
				return result;
			}

			auto registerResult = pResourceManager->RegisterFactory(pFactoryInstance);

			if (registerResult.HasError())
			{
				return registerResult.GetError();
			}
		}

		return RC_OK;
	}

	static E_RESULT_CODE RegisterResourceLoaders(IEngineCore* pEngineCore)
	{
		IResourceManager* pResourceManager = pEngineCore->GetSubsystem<IResourceManager>().Get();
		IGraphicsContext* pGraphicsContext = pEngineCore->GetSubsystem<IGraphicsContext>().Get();
		IFileSystem* pFileSystem = pEngineCore->GetSubsystem<IFileSystem>().Get();

		if (!pResourceManager || !pGraphicsContext || !pFileSystem)
		{
			return RC_FAIL;
		}

		auto registerLoader = [](IResourceManager* pResourceManager, IResourceLoader* pLoader) -> E_RESULT_CODE
		{
			auto registerResult = pResourceManager->RegisterLoader(pLoader);

			if (registerResult.HasError())
			{
				return registerResult.GetError();
			}

			return RC_OK;
		};

		E_RESULT_CODE result = RC_OK;

		auto pShaderCompilerInstance = TPtr<IShaderCompiler>(CreateD3D12ShaderCompiler(pFileSystem, pEngineCore->GetSubsystem<IPluginManager>()->GetDLLManager().Get(), result));

		if (result != RC_OK)
		{
			return result;
		}

		IResourceLoader* pLoaderInstance = CreateBaseShaderLoader(
			pResourceManager,
			pGraphicsContext,
			pFileSystem,
			pShaderCompilerInstance,
			pGraphicsContext->GetGraphicsObjectManager()->CreateShaderCache(pFileSystem).Get(),
			result);

		if (result != RC_OK || ((result = registerLoader(pResourceManager, pLoaderInstance)) != RC_OK))
		{
			return result;
		}

		return RC_OK;
	}


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

		mpGraphicsContext = TPtr<IGraphicsContext>(CreateD3D12GraphicsContext(pEngineCore->GetSubsystem<IWindowSystem>(), result));
		if (result != RC_OK)
		{
			return result;
		}
		
		if ((result = pEngineCore->RegisterSubsystem(DynamicPtrCast<IEngineSubsystem>(mpGraphicsContext))) != RC_OK)
		{
			return result;
		}

		if (RC_OK != (result = RegisterFactories(pEngineCore)))
		{
			return result;
		}

		if (RC_OK != (result = RegisterResourceLoaders(pEngineCore)))
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