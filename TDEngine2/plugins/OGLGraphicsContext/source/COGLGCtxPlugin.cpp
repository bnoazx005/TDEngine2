#include "./../include/COGLGCtxPlugin.h"
#include <core/IEngineCore.h>
#include <core/IGraphicsContext.h>
#include "./../include/COGLGraphicsContext.h"
#include "./../include/win32/CWin32GLContextFactory.h"
#include "./../include/unix/CUnixGLContextFactory.h"
#include <core/IWindowSystem.h>
#include <core/IFileSystem.h>


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

		IResourceFactory* pFactoryInstance = CreateOGLTexture2DFactory(mpGraphicsContext, result);

		if (result != RC_OK)
		{
			return result;
		}

		TRegisterFactoryResult registerResult = pResourceManager->RegisterFactory(pFactoryInstance);

		return registerResult.mResultCode;
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
		
		IResourceLoader* pLoaderInstance = CreateBaseTexture2DLoader(pResourceManager, mpGraphicsContext, pFileSystem, result);

		if (result != RC_OK)
		{
			return result;
		}

		TRegisterLoaderResult registerResult = pResourceManager->RegisterLoader(pLoaderInstance);

		if (registerResult.mResultCode != RC_OK)
		{
			return registerResult.mResultCode;
		}

		return registerResult.mResultCode;
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