#include "./../include/COGLGCtxPlugin.h"
#include <core/IEngineCore.h>
#include <core/IGraphicsContext.h>
#include "./../include/COGLGraphicsContext.h"
#include "./../include/win32/CWin32GLContextFactory.h"
#include "./../include/unix/CUnixGLContextFactory.h"
#include <core/IWindowSystem.h>
#include <core/IFileSystem.h>
#include <graphics/CBaseShaderLoader.h>


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
		
		mpEngineCoreInstance = pEngineCore;

		E_RESULT_CODE result = RC_OK;
		
		TCreateGLContextFactoryCallback pGLContextFactoryCallback = nullptr;

#if defined (TDE2_USE_WIN32PLATFORM)
		pGLContextFactoryCallback = CreateWin32GLContextFactory;
#elif defined (TDE2_USE_UNIXPLATFORM)
		pGLContextFactoryCallback = CreateUnixGLContextFactory;
#else
		/// \todo Implement callback assigment for other platforms
#endif

		mpGraphicsContext = CreateOGLGraphicsContext(dynamic_cast<IWindowSystem*>(pEngineCore->GetSubsystem(EST_WINDOW)), pGLContextFactoryCallback, result);

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

	E_RESULT_CODE COGLGCtxPlugin::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		delete this;

		mIsInitialized = false;

		return RC_OK;
	}

	const TPluginInfo& COGLGCtxPlugin::GetInfo() const
	{
		return mPluginInfo;
	}

	E_RESULT_CODE COGLGCtxPlugin::_registerFactories(IEngineCore* pEngineCore)
	{
		IResourceManager* pResourceManager = dynamic_cast<IResourceManager*>(pEngineCore->GetSubsystem(EST_RESOURCE_MANAGER));

		if (!pResourceManager)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		IResourceFactory* pFactoryInstance = CreateOGLShaderFactory(pResourceManager, mpGraphicsContext, result);

		if (result != RC_OK)
		{
			return result;
		}

		TResult<TResourceFactoryId> shaderFactoryResult = pResourceManager->RegisterFactory(pFactoryInstance);

		if (shaderFactoryResult.HasError())
		{
			return shaderFactoryResult.GetError();
		}

		pFactoryInstance = CreateOGLTexture2DFactory(pResourceManager, mpGraphicsContext, result);

		if (result != RC_OK)
		{
			return result;
		}

		TResult<TResourceFactoryId> textureFactoryResult = pResourceManager->RegisterFactory(pFactoryInstance);

		if (textureFactoryResult.HasError())
		{
			return textureFactoryResult.GetError();
		}

		return RC_OK;
	}

	E_RESULT_CODE COGLGCtxPlugin::_registerResourceLoaders(IEngineCore* pEngineCore)
	{
		IResourceManager* pResourceManager = dynamic_cast<IResourceManager*>(pEngineCore->GetSubsystem(EST_RESOURCE_MANAGER));

		IFileSystem* pFileSystem = dynamic_cast<IFileSystem*>(pEngineCore->GetSubsystem(EST_FILE_SYSTEM));

		if (!pResourceManager || !pFileSystem)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;
		
		IShaderCompiler* pShaderCompilerInstance = CreateOGLShaderCompiler(pFileSystem, result);

		if (result != RC_OK)
		{
			return result;
		}

		IResourceLoader* pLoaderInstance = CreateBaseShaderLoader(pResourceManager, mpGraphicsContext, pFileSystem, pShaderCompilerInstance, result);

		if (result != RC_OK)
		{
			return result;
		}

		TResult<TResourceLoaderId> shaderLoaderResult = pResourceManager->RegisterLoader(pLoaderInstance);

		if (shaderLoaderResult.HasError())
		{
			return shaderLoaderResult.GetError();
		}

		pLoaderInstance = CreateBaseTexture2DLoader(pResourceManager, mpGraphicsContext, pFileSystem, result);

		if (result != RC_OK)
		{
			return result;
		}

		TResult<TResourceLoaderId> textureLoaderResult = pResourceManager->RegisterLoader(pLoaderInstance);

		if (textureLoaderResult.HasError())
		{
			return textureLoaderResult.GetError();
		}

		return RC_OK;
	}
}


extern "C" TDE2_API TDEngine2::IPlugin* TDE2_APIENTRY CreatePlugin(TDEngine2::IEngineCore* pEngineCore, TDEngine2::E_RESULT_CODE& result)
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