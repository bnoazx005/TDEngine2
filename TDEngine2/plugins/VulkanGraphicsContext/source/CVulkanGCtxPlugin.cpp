#include "../include/CVulkanGCtxPlugin.h"
#include "../include/CVulkanGraphicsContext.h"
#include "../include/win32/CWin32WindowSurfaceFactory.h"
#include <core/IEngineCore.h>
#include <core/IGraphicsContext.h>
#include <core/IWindowSystem.h>
//#include <core/IFileSystem.h>
//#include <graphics/CBaseShaderLoader.h>


namespace TDEngine2
{
	CVulkanGCtxPlugin::CVulkanGCtxPlugin() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CVulkanGCtxPlugin::Init(IEngineCore* pEngineCore)
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
		
		auto pWindowSurfaceFactory = TPtr<IWindowSurfaceFactory>(
#if defined(TDE2_USE_WINPLATFORM)
			CreateWin32WindowSurfaceFactory(pEngineCore->GetSubsystem<IWindowSystem>().Get(), result)
#elif defined(TDE2_USE_UNIXPLATFORM)
			nullptr
#endif
			);

		mpGraphicsContext = TPtr<IGraphicsContext>(CreateVulkanGraphicsContext(pEngineCore->GetSubsystem<IWindowSystem>(), pWindowSurfaceFactory, result));
		if (result != RC_OK)
		{
			return result;
		}

		if ((result = pEngineCore->RegisterSubsystem(DynamicPtrCast<IEngineSubsystem>(mpGraphicsContext))) != RC_OK)
		{
			return result;
		}

		/*if ((result = _registerFactories(pEngineCore)) != RC_OK)
		{
			return result;
		}

		if ((result = _registerResourceLoaders(pEngineCore)) != RC_OK)
		{
			return result;
		}*/

		mIsInitialized = true;

		return RC_OK;
	}

	const TPluginInfo& CVulkanGCtxPlugin::GetInfo() const
	{
		static const TPluginInfo pluginInfo
		{
			"VulkanGraphicsContext",
			(0 << 16) | 6,
			(0 << 16) | 6,
		};

		return pluginInfo;
	}

	E_RESULT_CODE CVulkanGCtxPlugin::_registerFactories(IEngineCore* pEngineCore)
	{
		/*IResourceManager* pResourceManager = pEngineCore->GetSubsystem<IResourceManager>().Get();

		if (!pResourceManager)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		auto factoryFunctions = 
		{
			CreateVulkanShaderFactory,
			CreateVulkanTexture2DFactory,
			CreateVulkanCubemapTextureFactory,
			CreateVulkanRenderTargetFactory,
			CreateVulkanDepthBufferTargetFactory,
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
		}*/

		return RC_OK;
	}

	E_RESULT_CODE CVulkanGCtxPlugin::_registerResourceLoaders(IEngineCore* pEngineCore)
	{
		/*IResourceManager* pResourceManager = pEngineCore->GetSubsystem<IResourceManager>().Get();

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
		
		auto pShaderCompilerInstance = TPtr<IShaderCompiler>(CreateVulkanShaderCompiler(pFileSystem, result));

		if (result != RC_OK)
		{
			return result;
		}

		IResourceLoader* pLoaderInstance = CreateBaseShaderLoader(
			pResourceManager, 
			mpGraphicsContext.Get(), 
			pFileSystem, 
			pShaderCompilerInstance, 
			mpGraphicsContext->GetGraphicsObjectManager()->CreateShaderCache(pFileSystem).Get(),
			result);

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
		}*/

		return RC_OK;
	}
}


extern "C" TDE2_API TDEngine2::IPlugin* TDE2_APIENTRY CreatePlugin(TDEngine2::IEngineCore* pEngineCore, TDEngine2::E_RESULT_CODE& result)
{
	TDEngine2::CVulkanGCtxPlugin* pPluginInstance = new (std::nothrow) TDEngine2::CVulkanGCtxPlugin();

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