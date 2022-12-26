#include "../../include/ecs/CEntityManager.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/core/IEventManager.h"
#include "../../include/editor/CPerfProfiler.h"


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
		return _createEntity(_constructDefaultEntityName(mNextIdValue));
	}

	TPtr<CEntity> CEntityManager::Create(const std::string& name)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _createEntity(name);
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

		E_RESULT_CODE result = mpComponentManager->RemoveComponents(entityId);

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
		}

		mpEventManager->Notify(&onEntityRemoved);

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

	TPtr<CEntity> CEntityManager::_createEntity(const std::string& name)
	{
		E_RESULT_CODE result = RC_OK;

		TEntityId id = TEntityId(mNextIdValue);

		auto pEntity = TPtr<CEntity>(CreateEntity(id, name, this, result));
		
		if (result != RC_OK)
		{
			return nullptr;
		}

		size_t currStorageSize = mActiveEntities.size();

		if (mNextIdValue + 1 >= currStorageSize)
		{
			mActiveEntities.resize(currStorageSize + 1);
		}
		
		TOnEntityCreatedEvent onEntityCreated;

		onEntityCreated.mCreatedEntityId = id;

		mEntitiesHashTable[id] = mNextIdValue;

		mActiveEntities[mNextIdValue++] = pEntity;

		/// create basic component CTransform
		if (mCreateEntitiesWithPredefinedComponents)
		{
			CTransform* pTransform = pEntity->AddComponent<CTransform>();
			pTransform->SetOwnerId(id);
		}

		mpEventManager->Notify(&onEntityCreated);

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
		onComponentRemoved.mRemovedComponentTypeId = componentTypeId;

		mpEventManager->Notify(&onComponentRemoved);
	}
	

	CEntityManager* CreateEntityManager(IEventManager* pEventManager, IComponentManager* pComponentManager, bool createWithPredefinedComponents, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CEntityManager, CEntityManager, result, pEventManager, pComponentManager, createWithPredefinedComponents);
	}
}