#include "./../../include/ecs/CSystemManager.h"
#include "./../../include/ecs/ISystem.h"
#include "./../../include/ecs/IWorld.h"
#include "./../../include/ecs/CTransformSystem.h"
#include "./../../include/ecs/CSpriteRendererSystem.h"
#include "./../../include/utils/Utils.h"
#include "./../../include/core/IEventManager.h"
#include "./../../include/ecs/CEntity.h"
#include "./../../include/ecs/CBaseComponent.h"
#include <algorithm>


namespace TDEngine2
{
	CSystemManager::CSystemManager() :
		CBaseObject(), mpEventManager(nullptr), mpWorld(nullptr)
	{
	}

	E_RESULT_CODE CSystemManager::Init(IWorld* pWorld, IEventManager* pEventManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pEventManager || !pWorld)
		{
			return RC_INVALID_ARGS;
		}
		
		mpWorld = pWorld;

		mpEventManager = pEventManager;
		
		/// subscribe the manager onto events of ECS
		mpEventManager->Subscribe(TOnEntityCreatedEvent::GetTypeId(), this);
		mpEventManager->Subscribe(TOnEntityRemovedEvent::GetTypeId(), this);
		mpEventManager->Subscribe(TOnComponentCreatedEvent::GetTypeId(), this);
		mpEventManager->Subscribe(TOnComponentRemovedEvent::GetTypeId(), this);

		E_RESULT_CODE result = RC_OK;

		ISystem* pTransformSystem = CreateTransformSystem(result);

		if (result != RC_OK)
		{
			return result;
		}

		mBuiltinSystems.push_back(pTransformSystem);

		mIsInitialized = true;
		
		/// register builtin systems
		for (auto iter = mBuiltinSystems.begin(); iter != mBuiltinSystems.end(); ++iter)
		{
			RegisterSystem(*iter);
		}

		return RC_OK;
	}

	E_RESULT_CODE CSystemManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		ISystem* pCurrSystem = nullptr;

		for (auto iter = mBuiltinSystems.begin(); iter != mBuiltinSystems.end(); ++iter)
		{
			pCurrSystem = (*iter);

			if (!pCurrSystem || ((result = UnregisterSystemImmediately(pCurrSystem)) != RC_OK))
			{
				return result;
			}
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CSystemManager::RegisterSystem(ISystem* pSystem)
	{
		if (!pSystem)
		{
			return RC_INVALID_ARGS;
		}

		auto duplicateIter = std::find(mpActiveSystems.begin(), mpActiveSystems.end(), pSystem);

		if (duplicateIter != mpActiveSystems.end()) /// if there is a duplicate, just interrupt registration process
		{
			return RC_FAIL;
		}

		mpActiveSystems.push_back(pSystem);

		return RC_OK;
	}
	
	E_RESULT_CODE CSystemManager::UnregisterSystem(ISystem* pSystem)
	{
		if (!pSystem)
		{
			return RC_INVALID_ARGS;
		}

		auto targetSystemIter = std::find(mpActiveSystems.begin(), mpActiveSystems.end(), pSystem);

		if (targetSystemIter == mpActiveSystems.end()) /// specified system is not registred yet
		{
			return RC_FAIL;
		}

		mpActiveSystems.erase(targetSystemIter);

		return RC_OK;
	}

	E_RESULT_CODE CSystemManager::UnregisterSystemImmediately(ISystem* pSystem)
	{
		E_RESULT_CODE result = UnregisterSystem(pSystem);

		if (result != RC_OK)
		{
			return result;
		}

		return pSystem->Free();
	}

	E_RESULT_CODE CSystemManager::ActivateSystem(ISystem* pSystem)
	{
		auto tagretSystemIter = std::find(mpDeactivatedSystems.begin(), mpDeactivatedSystems.end(), pSystem);

		if (tagretSystemIter == mpDeactivatedSystems.end())
		{
			return RC_FAIL;
		}

		mpActiveSystems.push_back(pSystem);

		mpDeactivatedSystems.erase(tagretSystemIter);

		return RC_OK;
	}

	E_RESULT_CODE CSystemManager::DeactivateSystem(ISystem* pSystem)
	{
		auto targetSystemIter = std::find(mpActiveSystems.begin(), mpActiveSystems.end(), pSystem);

		if (targetSystemIter == mpActiveSystems.end())
		{
			return RC_FAIL;
		}

		mpDeactivatedSystems.push_back(pSystem);
		mpActiveSystems.erase(targetSystemIter);

		return RC_OK;
	}

	void CSystemManager::Update(IWorld* pWorld, float dt)
	{
		for (ISystem* pCurrSystem : mpActiveSystems)
		{
			pCurrSystem->Update(pWorld, dt);
		}
	}

	E_RESULT_CODE CSystemManager::OnEvent(const TBaseEvent* pEvent)
	{
		for (ISystem* pCurrSystem : mpActiveSystems)
		{
			pCurrSystem->InjectBindings(mpWorld);
		}

		return RC_OK;
	}

	TEventListenerId CSystemManager::GetListenerId() const
	{
		return GetTypeId();
	}

	ISystemManager* CreateSystemManager(IWorld* pWorld, IEventManager* pEventManager, E_RESULT_CODE& result)
	{
		CSystemManager* pSysManager = new (std::nothrow) CSystemManager();

		if (!pSysManager)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pSysManager->Init(pWorld, pEventManager);

		if (result != RC_OK)
		{
			delete pSysManager;

			pSysManager = nullptr;
		}

		return dynamic_cast<ISystemManager*>(pSysManager);
	}
}