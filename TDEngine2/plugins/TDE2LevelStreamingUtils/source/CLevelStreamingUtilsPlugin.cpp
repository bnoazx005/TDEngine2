#include "../include/CLevelStreamingUtilsPlugin.h"
#include "../include/CSceneChunksLoadingSystem.h"
#include "../include/CSceneLoadingTriggerComponent.h"
#include <core/IEngineCore.h>
#if TDE2_EDITORS_ENABLED
#include <editor/IEditorsManager.h>
#endif
#include <ecs/IWorld.h>
#include <scene/ISceneManager.h>


namespace TDEngine2
{
	CLevelStreamingUtilsPlugin::CLevelStreamingUtilsPlugin():
		CBaseObject()
	{
	}

	E_RESULT_CODE CLevelStreamingUtilsPlugin::Init(IEngineCore* pEngineCore)
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

		
		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CLevelStreamingUtilsPlugin::OnRegister(IEngineCore* pEngineCore, IWorld* pWorld)
	{
		E_RESULT_CODE result = RC_OK;

		auto systemRegistrationResult = pWorld->RegisterSystem(CreateSceneChunksLoadingSystem(pEngineCore->GetSubsystem<ISceneManager>().Get(), result));
		if (systemRegistrationResult.HasError())
		{
			return systemRegistrationResult.GetError();
		}

		result = result | pWorld->RegisterComponentFactory(TPtr<IComponentFactory>(CreateSceneLoadingTriggerComponentFactory(result)));
		if (RC_OK != result)
		{
			return result;
		}

#if TDE2_EDITORS_ENABLED
		if (auto pEditorsManager = pEngineCore->GetSubsystem<IEditorsManager>())
		{
			pEditorsManager->RegisterComponentInspector(CSceneLoadingTriggerComponent::GetTypeId(), CSceneLoadingTriggerComponent::DrawInspectorGUI);
		}
#endif

		return RC_OK;
	}

	const TPluginInfo& CLevelStreamingUtilsPlugin::GetInfo() const
	{
		static const TPluginInfo pluginInfo
		{
			"TDE2LevelStreamingUtils",
			(0 << 16) | 7,
			(0 << 16) | 7,
		};

		return pluginInfo;
	}
}


extern "C" TDE2_API TDEngine2::IPlugin* TDE2_APIENTRY CreatePlugin(TDEngine2::IEngineCore* pEngineCore, TDEngine2::E_RESULT_CODE& result)
{
	TDEngine2::CLevelStreamingUtilsPlugin* pPluginInstance = new (std::nothrow) TDEngine2::CLevelStreamingUtilsPlugin();

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