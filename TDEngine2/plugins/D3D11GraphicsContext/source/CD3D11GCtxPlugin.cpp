#include "../include/CD3D11GCtxPlugin.h"
#include "../include/CD3D11GraphicsContext.h"
#include "../include/CD3D11Shader.h"
#include "../include/CD3D11ShaderCompiler.h"
#include "../include/CD3D11Texture2D.h"
#include "../include/CD3D11RenderTarget.h"
#include <core/IEngineCore.h>
#include <core/IGraphicsContext.h>
#include <core/IWindowSystem.h>
#include <core/IResourceManager.h>
#include <core/IFileSystem.h>
#include <graphics/CBaseShaderLoader.h>
#include <graphics/CBaseTexture2D.h>


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

		mpGraphicsContext = TPtr<IGraphicsContext>(CreateD3D11GraphicsContext(pEngineCore->GetSubsystem<IWindowSystem>().Get(), result));

		if (result != RC_OK)
		{
			return result;
		}
		
		if ((result = pEngineCore->RegisterSubsystem(DynamicPtrCast<IEngineSubsystem>(mpGraphicsContext))) != RC_OK)
		{
			return result;
		}

		if ((result = _registerFactories(pEngineCore)) != RC_OK)
		{
			return result;
		}

		if ((result = _registerResourceLoaders(pEngineCore)) != RC_OK)
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

	E_RESULT_CODE CD3D11GCtxPlugin::_registerFactories(IEngineCore* pEngineCore)
	{
		IResourceManager* pResourceManager = pEngineCore->GetSubsystem<IResourceManager>().Get();

		if (!pResourceManager)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		auto factoryFunctions =
		{
			CreateD3D11ShaderFactory,
			CreateD3D11Texture2DFactory,
			CreateD3D11CubemapTextureFactory,
			CreateD3D11RenderTargetFactory,
			CreateD3D11DepthBufferTargetFactory,
		};

		IResourceFactory* pFactoryInstance = nullptr;

		for (auto currFactoryCallback : factoryFunctions)
		{
			pFactoryInstance = currFactoryCallback(pResourceManager, mpGraphicsContext.Get(), result);

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

	E_RESULT_CODE CD3D11GCtxPlugin::_registerResourceLoaders(IEngineCore* pEngineCore)
	{
		IResourceManager* pResourceManager = pEngineCore->GetSubsystem<IResourceManager>().Get();

		IFileSystem* pFileSystem = pEngineCore->GetSubsystem<IFileSystem>().Get();

		if (!pResourceManager || !pFileSystem)
		{
			return RC_FAIL;
		}

		auto registerLoader = [](IResourceManager* pResourceManager, const IResourceLoader* pLoader) -> E_RESULT_CODE
		{
			auto registerResult = pResourceManager->RegisterLoader(pLoader);

			if (registerResult.HasError())
			{
				return registerResult.GetError();
			}

			return RC_OK;
		};

		E_RESULT_CODE result = RC_OK;

		IShaderCompiler* pShaderCompilerInstance = CreateD3D11ShaderCompiler(pFileSystem, result);

		if (result != RC_OK)
		{
			return result;
		}

		IResourceLoader* pLoaderInstance = CreateBaseShaderLoader(pResourceManager, mpGraphicsContext.Get(), pFileSystem, pShaderCompilerInstance, result);

		if (result != RC_OK || ((result = registerLoader(pResourceManager, pLoaderInstance)) != RC_OK))
		{
			return result;
		}

		pLoaderInstance = CreateBaseTexture2DLoader(pResourceManager, mpGraphicsContext.Get(), pFileSystem, result);

		if (result != RC_OK || ((result = registerLoader(pResourceManager, pLoaderInstance)) != RC_OK))
		{
			return result;
		}

		pLoaderInstance = CreateBaseCubemapTextureLoader(pResourceManager, mpGraphicsContext.Get(), pFileSystem, result);

		if (result != RC_OK || ((result = registerLoader(pResourceManager, pLoaderInstance)) != RC_OK))
		{
			return result;
		}

		return RC_OK;
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