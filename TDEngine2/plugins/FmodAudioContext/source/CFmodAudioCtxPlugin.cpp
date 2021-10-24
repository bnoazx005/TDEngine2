#include "../include/CFmodAudioCtxPlugin.h"
#include "../include/CFmodAudioContext.h"
#include "../include/CFmodAudioListenerUpdateSystem.h"
#include <core/IEngineCore.h>
#include <core/IResourceManager.h>
#include <core/IAudioContext.h>
#include <core/IFileSystem.h>
#include <ecs/IWorld.h>


namespace TDEngine2
{
	CFMODAudioCtxPlugin::CFMODAudioCtxPlugin() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CFMODAudioCtxPlugin::Init(IEngineCore* pEngineCore)
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

		mpAudioContext = CreateFMODAudioContext(result);

		if (!mpAudioContext || result != RC_OK)
		{
			return result;
		}
		
		if (RC_OK != (result = pEngineCore->RegisterSubsystem(DynamicPtrCast<IEngineSubsystem>(mpAudioContext))))
		{
			return result;
		}

		result = result | _registerResourceFactories(pEngineCore);
		result = result | _registerResourceLoaders(pEngineCore);

		if (RC_OK != result)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CFMODAudioCtxPlugin::OnRegister(IEngineCore* pEngineCore, IWorld* pWorld)
	{
		if (!pWorld)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		std::vector<ISystem*> builtinSystems
		{
			CreateAudioListenerUpdateSystem(mpAudioContext.Get(), result),
			CreateAudioSourcesUpdateSystem(mpAudioContext.Get(), pEngineCore->GetSubsystem<IResourceManager>().Get(), result),
		};

		for (ISystem* pCurrSystem : builtinSystems)
		{
			if (!pCurrSystem)
			{
				continue;
			}

			pWorld->RegisterSystem(pCurrSystem);
		}

		return RC_OK;
	}

	const TPluginInfo& CFMODAudioCtxPlugin::GetInfo() const
	{
		static const TPluginInfo pluginInfo
		{
			"FmodAudioContext",
			(0 << 16) | 5,
			(0 << 16) | 5,
		};

		return pluginInfo;
	}

	E_RESULT_CODE CFMODAudioCtxPlugin::_registerResourceFactories(IEngineCore* pEngineCore)
	{
		IResourceManager* pResourceManager = pEngineCore->GetSubsystem<IResourceManager>().Get();

		if (!pResourceManager)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		auto factoryFunctions =
		{
			CreateFMODAudioClipFactory
		};

		IResourceFactory* pFactoryInstance = nullptr;

		for (auto currFactoryCallback : factoryFunctions)
		{
			pFactoryInstance = currFactoryCallback(pResourceManager, mpAudioContext.Get(), result);

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

	E_RESULT_CODE CFMODAudioCtxPlugin::_registerResourceLoaders(IEngineCore* pEngineCore)
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

		IResourceLoader* pLoaderInstance = CreateFMODAudioClipLoader(pResourceManager, mpAudioContext.Get(), pFileSystem, result);

		if (result != RC_OK || ((result = registerLoader(pResourceManager, pLoaderInstance)) != RC_OK))
		{
			return result;
		}

		return RC_OK;
	}
}


extern "C" TDE2_API TDEngine2::IPlugin* TDE2_APIENTRY CreatePlugin(TDEngine2::IEngineCore* pEngineCore, TDEngine2::E_RESULT_CODE& result)
{
	TDEngine2::CFMODAudioCtxPlugin* pPluginInstance = new (std::nothrow) TDEngine2::CFMODAudioCtxPlugin();

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