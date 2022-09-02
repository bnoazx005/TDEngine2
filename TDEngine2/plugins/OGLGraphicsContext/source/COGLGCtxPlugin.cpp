#include "../include/COGLGCtxPlugin.h"
#include "../include/COGLGraphicsContext.h"
#include "../include/win32/CWin32GLContextFactory.h"
#include "../include/unix/CUnixGLContextFactory.h"
#include <core/IEngineCore.h>
#include <core/IGraphicsContext.h>
#include <core/IWindowSystem.h>
#include <core/IFileSystem.h>
#include <graphics/CBaseShaderLoader.h>


namespace TDEngine2
{
	COGLGCtxPlugin::COGLGCtxPlugin() :
		CBaseObject()
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

#if defined (TDE2_USE_WINPLATFORM)
		pGLContextFactoryCallback = CreateWin32GLContextFactory;
#elif defined (TDE2_USE_UNIXPLATFORM)
		pGLContextFactoryCallback = CreateUnixGLContextFactory;
#else
		/// \todo Implement callback assigment for other platforms
#endif

		mpGraphicsContext = TPtr<IGraphicsContext>(CreateOGLGraphicsContext(pEngineCore->GetSubsystem<IWindowSystem>(), pGLContextFactoryCallback, result));

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

	const TPluginInfo& COGLGCtxPlugin::GetInfo() const
	{
		static const TPluginInfo pluginInfo
		{
			"OGLGraphicsContext",
			(0 << 16) | 5,
			(0 << 16) | 5,
		};

		return pluginInfo;
	}

	E_RESULT_CODE COGLGCtxPlugin::_registerFactories(IEngineCore* pEngineCore)
	{
		IResourceManager* pResourceManager = pEngineCore->GetSubsystem<IResourceManager>().Get();

		if (!pResourceManager)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		auto factoryFunctions = 
		{
			CreateOGLShaderFactory,
			CreateOGLTexture2DFactory,
			CreateOGLCubemapTextureFactory,
			CreateOGLRenderTargetFactory,
			CreateOGLDepthBufferTargetFactory,
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

	E_RESULT_CODE COGLGCtxPlugin::_registerResourceLoaders(IEngineCore* pEngineCore)
	{
		IResourceManager* pResourceManager = pEngineCore->GetSubsystem<IResourceManager>().Get();

		IFileSystem* pFileSystem = pEngineCore->GetSubsystem<IFileSystem>().Get();

		if (!pResourceManager || !pFileSystem)
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
		
		auto pShaderCompilerInstance = TPtr<IShaderCompiler>(CreateOGLShaderCompiler(pFileSystem, result));

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