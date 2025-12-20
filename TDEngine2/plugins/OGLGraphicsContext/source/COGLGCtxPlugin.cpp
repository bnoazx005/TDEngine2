#include "../include/COGLGCtxPlugin.h"
#include "../include/COGLGraphicsContext.h"
#include "../include/win32/CWin32GLContextFactory.h"
#include "../include/unix/CUnixGLContextFactory.h"
#include "../include/COGLResources.h"
#include <core/IEngineCore.h>
#include <core/IResourceManager.h>
#include <core/IGraphicsContext.h>
#include <core/IWindowSystem.h>
#include <core/IFileSystem.h>
#include <graphics/IGraphicsObjectManager.h>


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

		mpGraphicsContext = TPtr<IGraphicsContext>(CreateOGLGraphicsContext(pEngineCore->GetSubsystem<IWindowSystem>(), pEngineCore->GetSubsystem<IFileSystem>(), pGLContextFactoryCallback, result));

		if (result != RC_OK)
		{
			return result;
		}

		TPtr<IShaderCompiler> pShaderCompilerInstance = TPtr<IShaderCompiler>(CreateOGLShaderCompiler(pEngineCore->GetSubsystem<IFileSystem>().Get(), result));
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