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
		
		mIsInitialized = true;
		
		return RC_OK;
	}

	E_RESULT_CODE CSystemManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;
		
		while (!mpActiveSystems.empty())
		{
			UnregisterSystemImmediately(mpActiveSystems.front().mSystemId);
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	TResult<TSystemId> CSystemManager::RegisterSystem(ISystem* pSystem, E_SYSTEM_PRIORITY priority)
	{
		if (!pSystem)
		{
			return TErrorValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		auto duplicateIter = std::find_if(mpActiveSystems.begin(), mpActiveSystems.end(), [&pSystem](const TSystemDesc& sysDesc)
		{
			return sysDesc.mpSystem == pSystem;
		});

		if (duplicateIter != mpActiveSystems.end()) /// if there is a duplicate, just interrupt registration process
		{
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}

		U32 lastUsedSystemId = mSystemsIdentifiersTable[priority];

		++mSystemsIdentifiersTable[priority];

		/// low bytes contains id, high bytes - priority
		U32 internalSystemPriority = static_cast<U32>(priority) << 16 | lastUsedSystemId;

		mpActiveSystems.push_back({ internalSystemPriority, pSystem });

		return TOkValue<TSystemId>(internalSystemPriority);
	}
	
	E_RESULT_CODE CSystemManager::UnregisterSystem(TSystemId systemId)
	{
		return _internalUnregisterSystem(systemId);
	}

	E_RESULT_CODE CSystemManager::UnregisterSystemImmediately(TSystemId systemId)
	{
		ISystem* pSystem = _findSystemDesc(mpActiveSystems.begin(), mpActiveSystems.end(), systemId)->mpSystem;

		E_RESULT_CODE result = _internalUnregisterSystem(systemId);

		if (result != RC_OK)
		{
			return result;
		}

		return pSystem->Free();
	}

	E_RESULT_CODE CSystemManager::ActivateSystem(TSystemId systemId)
	{
		auto targetSystemIter = _findSystemDesc(mpDeactivatedSystems.begin(), mpDeactivatedSystems.end(), systemId);

		if (targetSystemIter == mpDeactivatedSystems.end())
		{
			return RC_FAIL;
		}

		mpActiveSystems.emplace_back(*targetSystemIter);

		mpDeactivatedSystems.erase(targetSystemIter);

		return RC_OK;
	}

	E_RESULT_CODE CSystemManager::DeactivateSystem(TSystemId systemId)
	{
		auto targetSystemIter = _findSystemDesc(mpActiveSystems.begin(), mpActiveSystems.end(), systemId);

		if (targetSystemIter == mpActiveSystems.end())
		{
			return RC_FAIL;
		}

		mpDeactivatedSystems.emplace_back(*targetSystemIter);
		mpActiveSystems.erase(targetSystemIter);

		return RC_OK;
	}

	void CSystemManager::Update(IWorld* pWorld, float dt)
	{
		ISystem* pCurrSystem = nullptr;

		for (auto currSystemDesc : mpActiveSystems)
		{
			pCurrSystem = currSystemDesc.mpSystem;

			pCurrSystem->Update(pWorld, dt);
		}
	}

	E_RESULT_CODE CSystemManager::OnEvent(const TBaseEvent* pEvent)
	{
		ISystem* pCurrSystem = nullptr;

		for (auto currSystemDesc : mpActiveSystems)
		{
			pCurrSystem = currSystemDesc.mpSystem;

			pCurrSystem->InjectBindings(mpWorld);
		}

		return RC_OK;
	}

	TEventListenerId CSystemManager::GetListenerId() const
	{
		return GetTypeId();
	}

	E_RESULT_CODE CSystemManager::_internalUnregisterSystem(TSystemId systemId)
	{
		if (systemId == InvalidSystemId)
		{
			return RC_INVALID_ARGS;
		}

		auto targetSystemIter = _findSystemDesc(mpActiveSystems.begin(), mpActiveSystems.end(), systemId);

		if (targetSystemIter == mpActiveSystems.end()) /// specified system is not registred yet
		{
			return RC_FAIL;
		}

		mpActiveSystems.erase(targetSystemIter);

		return RC_OK;
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