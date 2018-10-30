#include "./../../include/ecs/CComponentManager.h"
#include "./../../include/ecs/IComponentFactory.h"
#include "./../../include/ecs/IComponent.h"


namespace TDEngine2
{
	CComponentManager::CComponentManager() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CComponentManager::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CComponentManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CComponentManager::_removeComponent(TypeId componentTypeId, TEntityId entityId)
	{
		U32 targetEntityComponentHash = mEntityComponentMap[entityId][componentTypeId];

		if (targetEntityComponentHash == mInvalidMapValue)
		{
			return RC_FAIL;
		}

		IComponent* pComponent = mActiveComponents[--targetEntityComponentHash];

		if (!pComponent)
		{
			return RC_FAIL;
		}

		mActiveComponents[targetEntityComponentHash] = nullptr;

		mDestroyedComponents.push_back(pComponent);

		// mark handlers as invalid
		mEntityComponentMap[entityId][componentTypeId] = 0;
		mComponentEntityMap[componentTypeId][entityId] = 0;

		return RC_OK;
	}

	E_RESULT_CODE CComponentManager::_removeComponentImmediately(TypeId componentTypeId, TEntityId entityId)
	{
		U32 targetEntityComponentHash = mEntityComponentMap[entityId][componentTypeId];

		if (targetEntityComponentHash == mInvalidMapValue)
		{
			return RC_FAIL;
		}

		IComponent* pComponent = mActiveComponents[--targetEntityComponentHash];

		if (!pComponent)
		{
			return RC_FAIL;
		}

		mActiveComponents[targetEntityComponentHash] = nullptr;

		E_RESULT_CODE result = RC_OK;

		if ((result = pComponent->Free()) != RC_OK)
		{
			return result;
		}

		// mark handlers as invalid
		mEntityComponentMap[entityId][componentTypeId] = 0;
		mComponentEntityMap[componentTypeId][entityId] = 0;

		return RC_OK;
	}

	E_RESULT_CODE CComponentManager::RemoveComponents(TEntityId id)
	{
		auto entityComponentsListIter = mEntityComponentMap.find(id);

		if (entityComponentsListIter == mEntityComponentMap.end())
		{
			return RC_FAIL;
		}

		auto entityComponentsList = (*entityComponentsListIter).second;

		IComponent* pCurrComponent = nullptr;

		TComponentTypeId componentType = 0;
		U32 hashValue = 0;

		for (auto& currEntityComponentPair : entityComponentsList)
		{
			componentType = currEntityComponentPair.first;
			hashValue     = currEntityComponentPair.second - 1;
			
			pCurrComponent = mActiveComponents[hashValue];

			mActiveComponents[hashValue] = nullptr;

			mDestroyedComponents.push_back(pCurrComponent);
			
			// mark handlers as invalid
			mEntityComponentMap[id][componentType] = 0;
			mComponentEntityMap[componentType][id] = 0;
		}

		return RC_OK;
	}

	E_RESULT_CODE CComponentManager::RemoveComponentsImmediately(TEntityId id)
	{
		auto entityComponentsListIter = mEntityComponentMap.find(id);

		if (entityComponentsListIter == mEntityComponentMap.end())
		{
			return RC_FAIL;
		}

		auto entityComponentsList = (*entityComponentsListIter).second;

		IComponent* pCurrComponent = nullptr;

		TComponentTypeId componentType = 0;
		U32 hashValue = 0;

		E_RESULT_CODE result = RC_OK;

		for (auto& currEntityComponentPair : entityComponentsList)
		{
			componentType = currEntityComponentPair.first;
			hashValue = currEntityComponentPair.second - 1;

			pCurrComponent = mActiveComponents[hashValue];

			mActiveComponents[hashValue] = nullptr;

			// free memory
			if ((result = pCurrComponent->Free()) != RC_OK)
			{
				return result;
			}

			// mark handlers as invalid
			mEntityComponentMap[id][componentType] = 0;
			mComponentEntityMap[componentType][id] = 0;
		}
	}
	
	E_RESULT_CODE CComponentManager::RegisterFactory(const IComponentFactory* pFactory)
	{
		if (!pFactory)
		{
			return RC_INVALID_ARGS;
		}

		TypeId componentTypeId = pFactory->GetComponentTypeId();

		U32 hashValue = 0;

		if (mFreeComponentFactoriesRegistry.empty())
		{
			hashValue = mComponentFactories.size();

			mComponentFactories[hashValue] = pFactory;

			mComponentFactoriesMap[componentTypeId] = hashValue;

			return RC_OK;
		}

		hashValue = mFreeComponentFactoriesRegistry.front();

		mFreeComponentFactoriesRegistry.pop_front();
		
		mComponentFactories[hashValue] = pFactory;

		mComponentFactoriesMap[componentTypeId] = hashValue;

		return RC_OK;
	}

	E_RESULT_CODE CComponentManager::_unregisterFactory(TypeId typeId)
	{
		if (typeId == InvalidTypeId)
		{
			return RC_INVALID_ARGS;
		}
		
		TComponentFactoriesMap::const_iterator factoryIter = mComponentFactoriesMap.find(typeId);

		if (factoryIter == mComponentFactoriesMap.cend())
		{
			return RC_FAIL;
		}

		U32 hashValue = (*factoryIter).second;

		mComponentFactoriesMap.erase(factoryIter);

		mComponentFactories[hashValue] = nullptr;

		mFreeComponentFactoriesRegistry.push_front(hashValue);

		return RC_OK;
	}

	IComponent* CComponentManager::_createComponent(TypeId componentTypeId, TEntityId entityId)
	{
		IComponent* pNewComponent = _getComponent(componentTypeId, entityId);

		if (pNewComponent)
		{
			return pNewComponent;
		}

		TComponentFactoriesMap::const_iterator factoryIdIter = mComponentFactoriesMap.find(componentTypeId);

		if (factoryIdIter == mComponentFactoriesMap.cend())
		{
			return nullptr;
		}

		const IComponentFactory* pComponentFactory = mComponentFactories[(*factoryIdIter).second];

		pNewComponent = pComponentFactory->Create({});  /// \todo implement arguments passing

		mActiveComponents.push_back(pNewComponent);

		size_t hash = mActiveComponents.size();

		mComponentEntityMap[componentTypeId][entityId] = hash;
		mEntityComponentMap[entityId][componentTypeId] = hash;

		return pNewComponent;
	}

	IComponent* CComponentManager::_getComponent(TypeId componentTypeId, TEntityId entityId)
	{
		U32 hashValue = mComponentEntityMap[componentTypeId][entityId];

		if (hashValue == mInvalidMapValue)
		{
			return nullptr;
		}

		return mActiveComponents[hashValue - 1];
	}


	IComponentManager* CreateComponentManager(E_RESULT_CODE& result)
	{
		CComponentManager* pComponentManager = new (std::nothrow) CComponentManager();

		if (!pComponentManager)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pComponentManager->Init();

		if (result != RC_OK)
		{
			delete pComponentManager;

			pComponentManager = nullptr;
		}

		return dynamic_cast<IComponentManager*>(pComponentManager);
	}
}