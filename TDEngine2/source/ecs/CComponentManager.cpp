#include "./../../include/ecs/CComponentManager.h"


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


	CComponentManager* CreateComponentManager(E_RESULT_CODE& result)
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

		return pComponentManager;
	}
}