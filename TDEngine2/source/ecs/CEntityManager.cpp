#include "../../include/ecs/CEntityManager.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/core/IEventManager.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/editor/CStatsCounters.h"
#include <unordered_set>
#include "randomUtils.hpp"


namespace TDEngine2
{
	CEntityManager::CEntityManager() :
		CBaseObject(), mpEventManager(nullptr)
	{
	}

	E_RESULT_CODE CEntityManager::Init(IEventManager* pEventManager, IComponentManager* pComponentManager, bool createWithPredefinedComponents)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pComponentManager || !pEventManager)
		{
			return RC_INVALID_ARGS;
		}

		mpComponentManager = pComponentManager;
		mpEventManager = pEventManager;

		mCreateEntitiesWithPredefinedComponents = createWithPredefinedComponents;
		
		mNextIdValue = 0;
		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CEntityManager::_onFreeInternal()
	{
		return DestroyAllEntities();
	}

	TPtr<CEntity> CEntityManager::Create()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _createEntity(_constructDefaultEntityName(mNextIdValue++), GetNextUniqueIdentifier());
	}

	TPtr<CEntity> CEntityManager::Create(const std::string& name)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _createEntity(name, GetNextUniqueIdentifier());
	}

	TPtr<CEntity> CEntityManager::CreateWithUUID(TEntityId id)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _createEntity(_constructDefaultEntityName(mNextIdValue++), id);
	}

	E_RESULT_CODE CEntityManager::Destroy(TEntityId entityId)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _destroyInternal(entityId);
	}

	E_RESULT_CODE CEntityManager::_destroyInternal(TEntityId entityId, bool recomputeHandles)
	{
		if (TEntityId::Invalid == entityId)
		{
			return RC_INVALID_ARGS;
		}

		auto it = mEntitiesHashTable.find(entityId);
		if (it == mEntitiesHashTable.cend())
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		if (mCreateEntitiesWithPredefinedComponents)
		{
			if (auto pTransform = mpComponentManager->GetComponent<CTransform>(entityId)) /// \note Firstly remove all the children if they exist
			{
				for (const TEntityId currChildId : pTransform->GetChildren())
				{
					result = result | _destroyInternal(currChildId);
				}
			}
		}

		result = result | mpComponentManager->RemoveComponents(entityId, [this, entityId](auto&& removedComponents)
		{
			TOnComponentRemovedEvent onComponentsRemovedEvent;

			onComponentsRemovedEvent.mEntityId = entityId;
			onComponentsRemovedEvent.mRemovedComponentsTypeId = std::move(removedComponents);

			mpEventManager->Notify(&onComponentsRemovedEvent);
		});

		if (result != RC_OK)
		{
			return result;
		}

		TOnEntityRemovedEvent onEntityRemoved;

		{
			onEntityRemoved.mRemovedEntityId = entityId;

			const USIZE entityIndex = static_cast<USIZE>(it->second);

			mActiveEntities.erase(mActiveEntities.begin() + entityIndex);

			if (recomputeHandles) /// \note Update the table of handles for entities that're stored after the removed entity
			{
				for (auto&& currEntityInfo : mEntitiesHashTable)
				{
					if (currEntityInfo.second > entityIndex)
					{
						currEntityInfo.second--;
					}
				}
			}
		
			mEntitiesHashTable.erase(entityId);
			TDE2_ASSERT(mActiveEntities.size() == mEntitiesHashTable.size());
		}

		mpEventManager->Notify(&onEntityRemoved);

		TDE2_STATS_COUNTER_DECREMENT(mTotalEntitiesCount);

		return RC_OK;
	}

	E_RESULT_CODE CEntityManager::DestroyAllEntities()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		E_RESULT_CODE result = RC_OK;

		while (!mActiveEntities.empty())
		{
			auto& pEntity = mActiveEntities.back(); /// \note Remove from the back to prevent recomputation of handles 
			TDE2_ASSERT(pEntity);

			result = result | _destroyInternal(pEntity ? pEntity->GetId() : TEntityId::Invalid, false);
		}

		return result;
	}

	IComponent* CEntityManager::AddComponent(TEntityId entityId, TypeId componentTypeId)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		IComponent* pComponentInstance = mpComponentManager->CreateComponent(entityId, componentTypeId);

		_notifyOnAddComponent(entityId, componentTypeId);

		return pComponentInstance;
	}

	E_RESULT_CODE CEntityManager::RemoveComponent(TEntityId id, TypeId componentTypeId)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		E_RESULT_CODE result = mpComponentManager->RemoveComponent(componentTypeId, id);

		if (result != RC_OK)
		{
			return result;
		}

		_notifyOnRemovedComponent(id, componentTypeId);

		return result;
	}

	E_RESULT_CODE CEntityManager::RemoveComponents(TEntityId id)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mpComponentManager->RemoveComponents(id);
	}

	bool CEntityManager::HasComponent(TEntityId entityId, TypeId componentTypeId)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mpComponentManager->HasComponent(entityId, componentTypeId);
	}

	std::vector<IComponent*> CEntityManager::GetComponents(TEntityId id) const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mpComponentManager->GetComponents(id);
	}

	TPtr<CEntity> CEntityManager::GetEntity(TEntityId entityId) const
	{
		TDE2_PROFILER_SCOPE("CEntityManager::GetEntity");

		std::lock_guard<std::mutex> lock(mMutex);

		TEntitiesHashTable::const_iterator entityIter = mEntitiesHashTable.find(entityId);

		if (entityIter == mEntitiesHashTable.cend())
		{
			return nullptr;
		}

		return mActiveEntities[(*entityIter).second];
	}

	std::string CEntityManager::_constructDefaultEntityName(U32 id) const
	{
		return std::move(std::string("Entity").append(std::to_string(id)));
	}

	TPtr<CEntity> CEntityManager::_createEntity(const std::string& name, TEntityId id)
	{
		E_RESULT_CODE result = RC_OK;

		TDE2_ASSERT(mEntitiesHashTable.find(id) == mEntitiesHashTable.cend());

		auto pEntity = TPtr<CEntity>(CreateEntity(id, name, this, result));
		
		if (result != RC_OK)
		{
			return nullptr;
		}

		TOnEntityCreatedEvent onEntityCreated;

		onEntityCreated.mCreatedEntityId = id;

		mEntitiesHashTable[id] = static_cast<U32>(mActiveEntities.size());
		mActiveEntities.emplace_back(pEntity);

		/// create basic component CTransform
		if (mCreateEntitiesWithPredefinedComponents)
		{
			CTransform* pTransform = pEntity->AddComponent<CTransform>();
			pTransform->SetOwnerId(id);
		}

		mpEventManager->Notify(&onEntityCreated);

		TDE2_STATS_COUNTER_INCREMENT(mTotalEntitiesCount);

		return pEntity;
	}

	void CEntityManager::_notifyOnAddComponent(TEntityId entityId, TypeId componentTypeId)
	{
		TOnComponentCreatedEvent onComponentCreated;

		onComponentCreated.mEntityId               = entityId;
		onComponentCreated.mCreatedComponentTypeId = componentTypeId;

		mpEventManager->Notify(&onComponentCreated);
	}

	void CEntityManager::_notifyOnRemovedComponent(TEntityId entityId, TypeId componentTypeId)
	{
		TOnComponentRemovedEvent onComponentRemoved;

		onComponentRemoved.mEntityId               = entityId;
		onComponentRemoved.mRemovedComponentsTypeId.emplace_back(componentTypeId);

		mpEventManager->Notify(&onComponentRemoved);
	}
	

	CEntityManager* CreateEntityManager(IEventManager* pEventManager, IComponentManager* pComponentManager, bool createWithPredefinedComponents, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CEntityManager, CEntityManager, result, pEventManager, pComponentManager, createWithPredefinedComponents);
	}


	TEntityId GetNextUniqueIdentifier()
	{
		static Wrench::Random<U32, F32> randGenerator(static_cast<U32>(time(NULL)));
		static std::unordered_set<U32> generatedIdentifiers;

		U32 value = 0;

		do
		{
			value = randGenerator.Get(0, std::numeric_limits<U32>::max() - 1);
		} 
		while (generatedIdentifiers.find(value) != generatedIdentifiers.cend());

		generatedIdentifiers.emplace(value);

		return static_cast<TEntityId>(value);
	}
}