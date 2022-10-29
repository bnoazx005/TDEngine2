#include "../include/CBulletPhysicsPlugin.h"
#include "../include/CBoxCollisionObject3D.h"
#include "../include/CSphereCollisionObject3D.h"
#include "../include/CConvexHullCollisionObject3D.h"
#include "../include/CTrigger3D.h"
#include "../include/CPhysics3DSystem.h"
#include "../include/ComponentsInspectors.h"
#include <core/IEngineCore.h>
#include <core/IEventManager.h>
#if TDE2_EDITORS_ENABLED
#include <editor/IEditorsManager.h>
#endif
#include <ecs/IWorld.h>
#include <scene/ISceneManager.h>


namespace TDEngine2
{
	CBulletPhysicsPlugin::CBulletPhysicsPlugin():
		CBaseObject()
	{
	}

	E_RESULT_CODE CBulletPhysicsPlugin::Init(IEngineCore* pEngineCore)
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

	E_RESULT_CODE CBulletPhysicsPlugin::OnRegisterComponents(IEngineCore* pEngineCore, IWorld* pWorld)
	{
		E_RESULT_CODE result = RC_OK;

		auto componentsFactories =
		{
			CreateBoxCollisionObject3DFactory,
			CreateSphereCollisionObject3DFactory,
			CreateConvexHullCollisionObject3DFactory,
			CreateCapsuleCollisionObject3DFactory,
			CreateTrigger3DFactory,
		};

		for (auto pCurrFactoryCallback : componentsFactories)
		{
			TPtr<IComponentFactory> pCurrFactory = TPtr<IComponentFactory>(pCurrFactoryCallback(result));

			if (result != RC_OK)
			{
				return result;
			}

			if ((result = pWorld->RegisterComponentFactory(pCurrFactory)) != RC_OK)
			{
				return result;
			}
		}

#if TDE2_EDITORS_ENABLED
		result = result | RegisterComponentsInspector(pEngineCore->GetSubsystem<IEditorsManager>());
#endif

		return result;
	}

	E_RESULT_CODE CBulletPhysicsPlugin::OnRegisterSystems(IEngineCore* pEngineCore, IWorld* pWorld)
	{
		E_RESULT_CODE result = RC_OK;

		auto pPhysicsSystem = CreatePhysics3DSystem(pEngineCore->GetSubsystem<IEventManager>().Get(), result);
		pWorld->RegisterSystem(pPhysicsSystem);

		if (auto pRaycastContext = pWorld->GetRaycastContext())
		{
			result = result | pRaycastContext->Register3DPhysics(dynamic_cast<IPhysics3DSystem*>(pPhysicsSystem));
		}

		return result;
	}

	const TPluginInfo& CBulletPhysicsPlugin::GetInfo() const
	{
		static const TPluginInfo pluginInfo
		{
			"TDE2BulletPhysics",
			(0 << 16) | 7,
			(0 << 16) | 7,
		};

		return pluginInfo;
	}
}


extern "C" TDE2_API TDEngine2::IPlugin* TDE2_APIENTRY CreatePlugin(TDEngine2::IEngineCore* pEngineCore, TDEngine2::E_RESULT_CODE& result)
{
	TDEngine2::CBulletPhysicsPlugin* pPluginInstance = new (std::nothrow) TDEngine2::CBulletPhysicsPlugin();

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