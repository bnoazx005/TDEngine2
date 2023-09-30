#include "../../include/ecs/CSystemManager.h"
#include "../../include/ecs/ISystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CTransformSystem.h"
#include "../../include/ecs/CSpriteRendererSystem.h"
#include "../../include/utils/Utils.h"
#include "../../include/core/IEventManager.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CBaseComponent.h"
#include <algorithm>


namespace TDEngine2
{
	CSystemManager::CSystemManager() :
		CBaseObject(), mpEventManager(nullptr), mpWorld(nullptr)
	{
	}

	E_RESULT_CODE CSystemManager::Init(IWorld* pWorld, IEventManager* pEventManager, TPtr<IJobManager> pJobManager)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pEventManager || !pWorld || !pJobManager)
		{
			return RC_INVALID_ARGS;
		}
		
		mpWorld = pWorld;

		mpEventManager = pEventManager;
		mpJobManager = pJobManager;

		/// subscribe the manager onto events of ECS
		mpEventManager->Subscribe(TOnEntityCreatedEvent::GetTypeId(), this);
		mpEventManager->Subscribe(TOnEntityRemovedEvent::GetTypeId(), this);
		mpEventManager->Subscribe(TOnComponentCreatedEvent::GetTypeId(), this);
		mpEventManager->Subscribe(TOnComponentRemovedEvent::GetTypeId(), this);
		mpEventManager->Subscribe(TOnHierarchyChangedEvent::GetTypeId(), this);
		
		mIsDirty = true;

		mIsInitialized = true;
		
		return RC_OK;
	}

	E_RESULT_CODE CSystemManager::_onFreeInternal()
	{
		return ClearSystemsRegistry();
	}

	TResult<TSystemId> CSystemManager::RegisterSystem(ISystem* pSystem, E_SYSTEM_PRIORITY priority)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!pSystem)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		/// inject bindings for the first time when the system is registered
		pSystem->OnInit(mpJobManager);
		pSystem->InjectBindings(mpWorld);

		auto duplicateIter = std::find_if(mpActiveSystems.begin(), mpActiveSystems.end(), [&pSystem](const TSystemDesc& sysDesc)
		{
			return sysDesc.mpSystem == pSystem;
		});

		if (duplicateIter != mpActiveSystems.end()) /// if there is a duplicate, just interrupt registration process
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		U32 lastUsedSystemId = mSystemsIdentifiersTable[priority];

		++mSystemsIdentifiersTable[priority];

		/// low bytes contains id, high bytes - priority
		TSystemId internalSystemPriority = TSystemId(static_cast<U32>(priority) << 16 | lastUsedSystemId);

		mpActiveSystems.push_back({ internalSystemPriority, pSystem });

		return Wrench::TOkValue<TSystemId>(TSystemId(internalSystemPriority));
	}
	
	E_RESULT_CODE CSystemManager::UnregisterSystem(TSystemId systemId)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return _internalUnregisterSystem(systemId);
	}

	E_RESULT_CODE CSystemManager::UnregisterSystemImmediately(TSystemId systemId)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return _internalUnregisterSystemImmediately(systemId);
	}

	E_RESULT_CODE CSystemManager::ActivateSystem(TSystemId systemId)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		auto targetSystemIter = _findSystemDesc(mpDeactivatedSystems.begin(), mpDeactivatedSystems.end(), systemId);

		if (targetSystemIter == mpDeactivatedSystems.end())
		{
			return RC_FAIL;
		}

		if (ISystem* pSystem = targetSystemIter->mpSystem)
		{
			pSystem->OnActivated();
			pSystem->InjectBindings(mpWorld);
		}

		mpActiveSystems.emplace_back(*targetSystemIter);

		mpDeactivatedSystems.erase(targetSystemIter);

		return RC_OK;
	}

	E_RESULT_CODE CSystemManager::DeactivateSystem(TSystemId systemId)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		auto targetSystemIter = _findSystemDesc(mpActiveSystems.begin(), mpActiveSystems.end(), systemId);

		if (targetSystemIter == mpActiveSystems.end())
		{
			return RC_FAIL;
		}

		if (ISystem* pSystem = targetSystemIter->mpSystem)
		{
			pSystem->OnDeactivated();
		}

		mpDeactivatedSystems.emplace_back(*targetSystemIter);
		mpActiveSystems.erase(targetSystemIter);

		return RC_OK;
	}

	E_RESULT_CODE CSystemManager::InitSystems()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		ISystem* pCurrSystem = nullptr;

		for (auto currSystemDesc : mpActiveSystems)
		{
			pCurrSystem = currSystemDesc.mpSystem;

			pCurrSystem->OnInit(mpJobManager);
			pCurrSystem->OnActivated();
		}

		return RC_OK;
	}

	E_RESULT_CODE CSystemManager::ClearSystemsRegistry()
	{
		E_RESULT_CODE result = DestroySystems();

		std::lock_guard<std::mutex> lock(mMutex);

		while (!mpActiveSystems.empty())
		{
			result = result | _internalUnregisterSystemImmediately(mpActiveSystems.front().mSystemId);
		}

		while (!mpDeactivatedSystems.empty())
		{
			if (auto pSystem = mpDeactivatedSystems.front().mpSystem)
			{
				pSystem->Free();
			}

			mpDeactivatedSystems.pop_front();
		}

		return result;
	}

	void CSystemManager::Update(IWorld* pWorld, float dt)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		ISystem* pCurrSystem = nullptr;

		for (auto&& currSystemDesc : mpActiveSystems)
		{
			pCurrSystem = currSystemDesc.mpSystem;

			if (mIsDirty)
			{
				pCurrSystem->InjectBindings(mpWorld);
			}

			pCurrSystem->Update(pWorld, dt);

#if TDE2_EDITORS_ENABLED
#endif
		}

		mIsDirty = false;

		/// \note Execute all deffered commands after all updates
		for (auto currSystemDesc : mpActiveSystems)
		{
			pCurrSystem = currSystemDesc.mpSystem;
			pCurrSystem->ExecuteDefferedCommands();
		}
	}

	E_RESULT_CODE CSystemManager::DestroySystems()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		auto&& preDestroySystem = [this](auto&& container)
		{
			ISystem* pCurrSystem = nullptr;

			for (auto currSystemDesc : container)
			{
				pCurrSystem = currSystemDesc.mpSystem;

				pCurrSystem->OnDeactivated();
				pCurrSystem->OnDestroy();
			}
		};

		preDestroySystem(mpActiveSystems);
		preDestroySystem(mpDeactivatedSystems);

		return RC_OK;
	}

	E_RESULT_CODE CSystemManager::OnEvent(const TBaseEvent* pEvent)
	{
		mIsDirty = true;
		return RC_OK;
	}

	TEventListenerId CSystemManager::GetListenerId() const
	{
		return TEventListenerId(GetTypeId());
	}

	void CSystemManager::ForEachSystem(const std::function<void(TSystemId systemId, const ISystem* const pSystem)> action) const
	{
		TDE2_ASSERT(action);
		if (!action)
		{
			return;
		}

		for (auto&& currSystem : mpActiveSystems)
		{
			action(currSystem.mSystemId, currSystem.mpSystem);
		}

		for (auto&& currSystem : mpDeactivatedSystems)
		{
			action(currSystem.mSystemId, currSystem.mpSystem);
		}
	}

	bool CSystemManager::IsSystemActive(TSystemId systemId) const
	{
		return std::find_if(mpActiveSystems.cbegin(), mpActiveSystems.cend(), [systemId](auto&& systemInfoEntity) { return systemId == systemInfoEntity.mSystemId; }) != mpActiveSystems.cend();
	}

	E_RESULT_CODE CSystemManager::_internalUnregisterSystem(TSystemId systemId)
	{
		if (systemId == TSystemId::Invalid)
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

	E_RESULT_CODE CSystemManager::_internalUnregisterSystemImmediately(TSystemId systemId)
	{
		ISystem* pSystem = _findSystemDesc(mpActiveSystems.begin(), mpActiveSystems.end(), systemId)->mpSystem;

		E_RESULT_CODE result = _internalUnregisterSystem(systemId);

		if (result != RC_OK)
		{
			return result;
		}

		return pSystem->Free();
	}

	TSystemId CSystemManager::FindSystem(TypeId typeId)
	{
		auto iter = std::find_if(mpActiveSystems.cbegin(), mpActiveSystems.cend(), [typeId](const TSystemDesc& systemDesc)
		{
			return systemDesc.mpSystem->GetSystemType() == typeId;
		});

		return (iter == mpActiveSystems.cend()) ? TSystemId::Invalid : iter->mSystemId;
	}


	TPtr<ISystem> CSystemManager::GetSystem(TSystemId handle)
	{
		ISystem* pSystem = _findSystemDesc(mpActiveSystems.begin(), mpActiveSystems.end(), handle)->mpSystem;
		return MakeScopedFromRawPtr<ISystem>(pSystem);
	}


	ISystemManager* CreateSystemManager(IWorld* pWorld, IEventManager* pEventManager, TPtr<IJobManager> pJobManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystemManager, CSystemManager, result, pWorld, pEventManager, pJobManager);
	}
}