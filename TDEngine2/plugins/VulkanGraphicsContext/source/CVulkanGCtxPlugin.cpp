#include "../include/CVulkanGCtxPlugin.h"
#include "../include/CVulkanGraphicsContext.h"
#include "../include/win32/CWin32WindowSurfaceFactory.h"
#include "../include/unix/CUnixWindowSurfaceFactory.h"
#include "../include/CVulkanResources.h"
#include "../include/CVulkanShaderCompiler.h"
#include <core/IEngineCore.h>
#include <core/IGraphicsContext.h>
#include <core/IWindowSystem.h>
#include <core/IResourceManager.h>
#include <core/IPluginManager.h>
#include <graphics/IGraphicsObjectManager.h>


namespace TDEngine2
{
	TDE2_DEFINE_SCOPED_PTR(CVulkanBuffer)


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
			CreateUnixWindowSurfaceFactory(pEngineCore->GetSubsystem<IWindowSystem>().Get(), result)
#endif
			);

		mpGraphicsContext = TPtr<IGraphicsContext>(CreateVulkanGraphicsContext(pEngineCore->GetSubsystem<IWindowSystem>(), pEngineCore->GetSubsystem<IFileSystem>(), pWindowSurfaceFactory, result));
		if (result != RC_OK)
		{
			return result;
		}

		TPtr<IShaderCompiler> pShaderCompilerInstance = TPtr<IShaderCompiler>(CreateVulkanShaderCompiler(pEngineCore->GetSubsystem<IFileSystem>().Get(), pEngineCore->GetSubsystem<IPluginManager>()->GetDLLManager().Get(), result));
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