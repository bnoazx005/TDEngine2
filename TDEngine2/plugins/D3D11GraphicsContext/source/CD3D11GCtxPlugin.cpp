#include "./../include/CD3D11GCtxPlugin.h"
#include <core/IEngineCore.h>
#include <core/IGraphicsContext.h>
#include "./../include/CD3D11GraphicsContext.h"
#include <core/IWindowSystem.h>
#include <core/IResourceManager.h>
#include "./../include/CD3D11Shader.h"
#include <graphics/CBaseShaderLoader.h>
#include <core/IFileSystem.h>
#include "./../include/CD3D11ShaderCompiler.h"
#include <graphics/CBaseTexture2D.h>
#include "./../include/CD3D11Texture2D.h"
#include "./../include/CD3D11RenderTarget.h"


namespace TDEngine2
{
	CD3D11GCtxPlugin::CD3D11GCtxPlugin() :
		mIsInitialized(false)
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

		mpGraphicsContext = CreateD3D11GraphicsContext(dynamic_cast<IWindowSystem*>(pEngineCore->GetSubsystem(EST_WINDOW)), result);

		if (result != RC_OK)
		{
			return result;
		}
		
		if ((result = pEngineCore->RegisterSubsystem(mpGraphicsContext)) != RC_OK)
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

	E_RESULT_CODE CD3D11GCtxPlugin::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		delete this;

		mIsInitialized = false;

		return RC_OK;
	}

	const TPluginInfo& CD3D11GCtxPlugin::GetInfo() const
	{
		return mPluginInfo;
	}

	E_RESULT_CODE CD3D11GCtxPlugin::_registerFactories(IEngineCore* pEngineCore)
	{
		IResourceManager* pResourceManager = dynamic_cast<IResourceManager*>(pEngineCore->GetSubsystem(EST_RESOURCE_MANAGER));

		if (!pResourceManager)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		/// Register a factory of shaders
		IResourceFactory* pFactoryInstance = CreateD3D11ShaderFactory(mpGraphicsContext, result);

		if (result != RC_OK)
		{
			return result;
		}

		TRegisterFactoryResult registerResult = pResourceManager->RegisterFactory(pFactoryInstance);

		if (registerResult.mResultCode != RC_OK)
		{
			return registerResult.mResultCode;
		}

		/// Register a factory of 2D textures
		pFactoryInstance = CreateD3D11Texture2DFactory(mpGraphicsContext, result);

		if (result != RC_OK)
		{
			return result;
		}

		registerResult = pResourceManager->RegisterFactory(pFactoryInstance);

		/// Try to register a factory of render target objects
		pFactoryInstance = CreateD3D11RenderTargetFactory(mpGraphicsContext, result);

		if (result != RC_OK)
		{
			return result;
		}

		registerResult = pResourceManager->RegisterFactory(pFactoryInstance);

		return registerResult.mResultCode;
	}

	E_RESULT_CODE CD3D11GCtxPlugin::_registerResourceLoaders(IEngineCore* pEngineCore)
	{
		IResourceManager* pResourceManager = dynamic_cast<IResourceManager*>(pEngineCore->GetSubsystem(EST_RESOURCE_MANAGER));

		IFileSystem* pFileSystem = dynamic_cast<IFileSystem*>(pEngineCore->GetSubsystem(EST_FILE_SYSTEM));

		if (!pResourceManager || !pFileSystem)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		IShaderCompiler* pShaderCompilerInstance = CreateD3D11ShaderCompiler(pFileSystem, result);

		if (result != RC_OK)
		{
			return result;
		}

		IResourceLoader* pLoaderInstance = CreateBaseShaderLoader(pResourceManager, mpGraphicsContext, pFileSystem, pShaderCompilerInstance, result);
		
		if (result != RC_OK)
		{
			return result;
		}

		TRegisterLoaderResult registerResult = pResourceManager->RegisterLoader(pLoaderInstance);

		if (registerResult.mResultCode != RC_OK)
		{
			return registerResult.mResultCode;
		}

		pLoaderInstance = CreateBaseTexture2DLoader(pResourceManager, mpGraphicsContext, pFileSystem, result);

		if (result != RC_OK)
		{
			return result;
		}

		registerResult = pResourceManager->RegisterLoader(pLoaderInstance);

		return registerResult.mResultCode;
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