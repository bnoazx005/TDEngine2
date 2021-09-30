#include "../../include/ecs/CEntityManager.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/core/IEventManager.h"


namespace TDEngine2
{
	CEntityManager::CEntityManager() :
		CBaseObject(), mpEventManager(nullptr)
	{
	}

	E_RESULT_CODE CEntityManager::Init(IEventManager* pEventManager, IComponentManager* pComponentManager)
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

		mNextIdValue = 0;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CEntityManager::_onFreeInternal()
	{
		return DestroyAllImmediately();
	}

	CEntity* CEntityManager::Create()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _createEntity(_constructDefaultEntityName(mNextIdValue));
	}

	CEntity* CEntityManager::Create(const std::string& name)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _createEntity(name);
	}

	E_RESULT_CODE CEntityManager::Destroy(CEntity* pEntity)
	{
		if (!pEntity)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = mpComponentManager->RemoveComponents(pEntity->GetId());

		if (result != RC_OK)
		{
			return result;
		}

		TOnEntityRemovedEvent onEntityRemoved;

		{
			std::lock_guard<std::mutex> lock(mMutex);

			TEntityId id = pEntity->GetId();

			onEntityRemoved.mRemovedEntityId = id;

			mActiveEntities[static_cast<U32>(id)] = nullptr;

			mDestroyedEntities.push_back(pEntity);

			mEntitiesHashTable.erase(id);
		}

		mpEventManager->Notify(&onEntityRemoved);

		return RC_OK;
	}

	E_RESULT_CODE CEntityManager::DestroyImmediately(CEntity* pEntity)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return _destroyImmediatelyInternal(pEntity);
	}

	E_RESULT_CODE CEntityManager::DestroyAllEntities()
	{
		std::lock_guard<std::mutex> lock(mMutex);

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
		std::lock_guard<std::mutex> lock(mMutex);

		E_RESULT_CODE result = RC_OK;

		for (CEntity* pEntity : mActiveEntities)
		{
			if (!pEntity)
			{
				continue;
			}

			if ((result = _destroyImmediatelyInternal(pEntity)) != RC_OK)
			{
				return result;
			}
		}

		return RC_OK;
	}

	IComponent* CEntityManager::AddComponent(TEntityId entityId, TypeId componentTypeId)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		IComponent* pComponentInstance = mpComponentManager->CreateComponent(entityId, componentTypeId);

		_notifyOnAddComponent(entityId, componentTypeId);

		return pComponentInstance;
	}

	E_RESULT_CODE CEntityManager::RemoveComponents(TEntityId id)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mpComponentManager->RemoveComponents(id);
	}

	std::vector<IComponent*> CEntityManager::GetComponents(TEntityId id) const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mpComponentManager->GetComponents(id);
	}

	CEntity* CEntityManager::GetEntity(TEntityId entityId) const
	{
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
		CTransform* pTransform = pEntity->AddComponent<CTransform>();
		pTransform->SetOwnerId(id);

		mpEventManager->Notify(&onEntityCreated);

		return pEntity;
	}

	E_RESULT_CODE CEntityManager::_destroyImmediatelyInternal(CEntity* pEntity)
	{
		if (!pEntity)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = mpComponentManager->RemoveComponents(pEntity->GetId());

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
		return CREATE_IMPL(CEntityManager, CEntityManager, result, pEventManager, pComponentManager);
	}
}