#include "./../../include/ecs/CEntityManager.h"
#include "./../../include/ecs/CEntity.h"
#include "./../../include/ecs/CTransform.h"
#include "./../../include/core/IEventManager.h"


namespace TDEngine2
{
	CEntityManager::CEntityManager() :
		CBaseObject(), mpEventManager(nullptr)
	{
	}

	E_RESULT_CODE CEntityManager::Init(IEventManager* pEventManager, IComponentManager* pComponentManager)
	{
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

		mNextIdValue = 0;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CEntityManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = DestroyAllImmediately();

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	CEntity* CEntityManager::Create()
	{
		return _createEntity(_constructDefaultEntityName(mNextIdValue));
	}

	CEntity* CEntityManager::Create(const std::string& name)
	{
		return _createEntity(name);
	}

	E_RESULT_CODE CEntityManager::Destroy(CEntity* pEntity)
	{
		if (!pEntity)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = pEntity->RemoveComponents();

		if (result != RC_OK)
		{
			return result;
		}
		
		TEntityId id = pEntity->GetId();

		TOnEntityRemovedEvent onEntityRemoved;
		
		onEntityRemoved.mRemovedEntityId = id;

		mActiveEntities[static_cast<U32>(id)] = nullptr;

		mDestroyedEntities.push_back(pEntity);

		mEntitiesHashTable.erase(id);

		mpEventManager->Notify(&onEntityRemoved);

		return RC_OK;
	}

	E_RESULT_CODE CEntityManager::DestroyImmediately(CEntity* pEntity)
	{
		if (!pEntity)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = pEntity->RemoveComponents();

		if (result != RC_OK)
		{
			return result;
		}

		TEntityId id = pEntity->GetId();

		TOnEntityRemovedEvent onEntityRemoved;

		onEntityRemoved.mRemovedEntityId = id;

		result = pEntity->Free();

		if (result != RC_OK)
		{
			return result;
		}
				
		mActiveEntities[static_cast<U32>(id)] = nullptr;

		mEntitiesHashTable.erase(id);
		
		mpEventManager->Notify(&onEntityRemoved);

		return RC_OK;
	}

	E_RESULT_CODE CEntityManager::DestroyAllEntities()
	{
		E_RESULT_CODE result = RC_OK;

		for (CEntity* pEntity : mActiveEntities)
		{
			if (!pEntity)
			{
				continue;
			}

			if ((result = Destroy(pEntity)) != RC_OK)
			{
				return result;
			}
		}

		return RC_OK;
	}

	E_RESULT_CODE CEntityManager::DestroyAllImmediately()
	{
		E_RESULT_CODE result = RC_OK;

		for (CEntity* pEntity : mActiveEntities)
		{
			if (!pEntity)
			{
				continue;
			}

			if ((result = DestroyImmediately(pEntity)) != RC_OK)
			{
				return result;
			}
		}

		return RC_OK;
	}

	E_RESULT_CODE CEntityManager::RemoveComponents(TEntityId id)
	{
		return mpComponentManager->RemoveComponents(id);
	}

	CEntity* CEntityManager::GetEntity(TEntityId entityId) const
	{
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

	CEntity* CEntityManager::_createEntity(const std::string& name)
	{
		E_RESULT_CODE result = RC_OK;

		TEntityId id = TEntityId(mNextIdValue);

		CEntity* pEntity = CreateEntity(id, name, this, result);
		
		if (result != RC_OK)
		{
			return nullptr;
		}

		size_t currStorageSize = mActiveEntities.size();

		if (mNextIdValue + 1 >= currStorageSize)
		{
			mActiveEntities.resize(currStorageSize + 10); /// \todo 10 is just a magic constant that should be replaced in some way
		}
		
		TOnEntityCreatedEvent onEntityCreated;

		onEntityCreated.mCreatedEntityId = id;

		mEntitiesHashTable[id] = mNextIdValue;

		mActiveEntities[mNextIdValue++] = pEntity;

		/// create basic component CTransform
		pEntity->AddComponent<CTransform>();
		
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
	

	CEntityManager* CreateEntityManager(IEventManager* pEventManager, IComponentManager* pComponentManager, E_RESULT_CODE& result)
	{
		CEntityManager* pEntityManager = new (std::nothrow) CEntityManager();

		if (!pEntityManager)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pEntityManager->Init(pEventManager, pComponentManager);

		if (result != RC_OK)
		{
			delete pEntityManager;

			pEntityManager = nullptr;
		}

		return pEntityManager;
	}
}