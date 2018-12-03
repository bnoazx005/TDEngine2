#include "./../../include/ecs/CComponentManager.h"
#include "./../../include/ecs/IComponentFactory.h"
#include "./../../include/ecs/IComponent.h"
#include "./../../include/ecs/CTransform.h"
#include "./../../include/graphics/CQuadSprite.h"
#include "./../../include/graphics/CPerspectiveCamera.h"
#include "./../../include/graphics/COrthoCamera.h"


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

		return _registerBuiltinComponentFactories();
	}

	E_RESULT_CODE CComponentManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		if ((result = _unregisterBuiltinComponentFactories()))
		{
			return result;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CComponentManager::_removeComponent(TypeId componentTypeId, TEntityId entityId)
	{
		IComponent* pRemovedComponent = nullptr;

		E_RESULT_CODE result = _removeComponentWithAction(componentTypeId, entityId, [&pRemovedComponent](IComponent*& pComponent) -> E_RESULT_CODE
		{
			pRemovedComponent = pComponent;

			return RC_OK;
		});

		if (result != RC_OK)
		{
			return result;
		}

		mDestroyedComponents.push_back(pRemovedComponent);

		return result;
	}

	E_RESULT_CODE CComponentManager::_removeComponentImmediately(TypeId componentTypeId, TEntityId entityId)
	{
		return _removeComponentWithAction(componentTypeId, entityId, [](IComponent*& pComponent) -> E_RESULT_CODE
		{
			return pComponent->Free();
		});
	}


	E_RESULT_CODE CComponentManager::_removeComponentWithAction(TypeId componentTypeId, TEntityId entityId,
																const std::function<E_RESULT_CODE(IComponent*&)>& action)
	{
		U32 targetEntityComponentHash = mComponentEntityMap[componentTypeId][entityId];

		if (!targetEntityComponentHash || (mComponentsHashTable.find(componentTypeId) == mComponentsHashTable.cend()))
		{
			return RC_FAIL;
		}

		U32 componentHashValue = mComponentsHashTable[componentHashValue];

		IComponent* pComponent = mActiveComponents[componentHashValue][targetEntityComponentHash - 1];

		if (!pComponent)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = action(pComponent);

		if (result != RC_OK)
		{
			return result;
		}

		mActiveComponents[targetEntityComponentHash][targetEntityComponentHash - 1] = nullptr;
		
		// mark handlers as invalid
		mComponentEntityMap[componentTypeId][entityId] = 0;
		mEntityComponentMap[entityId][componentTypeId] = 0;

		return RC_OK;
	}

	E_RESULT_CODE CComponentManager::RemoveComponents(TEntityId id)
	{
		std::vector<IComponent*> removedComponents;

		E_RESULT_CODE result = _removeComponentsWithAction(id, [&removedComponents](IComponent*& pComponent) -> E_RESULT_CODE
		{
			removedComponents.push_back(pComponent);

			return RC_OK;
		});

		if (result != RC_OK)
		{
			return result;
		}

		mDestroyedComponents.insert(mDestroyedComponents.end(), removedComponents.begin(), removedComponents.end());

		return result;
	}

	E_RESULT_CODE CComponentManager::RemoveComponentsImmediately(TEntityId id)
	{
		return _removeComponentsWithAction(id, [](IComponent*& pComponent) -> E_RESULT_CODE
		{
			return pComponent->Free();
		});
	}

	E_RESULT_CODE CComponentManager::_removeComponentsWithAction(TEntityId entityId, const std::function<E_RESULT_CODE(IComponent*&)>& action)
	{
		auto entityComponentsListIter = mEntityComponentMap.find(entityId);

		if (entityComponentsListIter == mEntityComponentMap.end())
		{
			return RC_FAIL;
		}

		auto entityComponentsList = (*entityComponentsListIter).second;

		IComponent* pCurrComponent = nullptr;

		TComponentTypeId componentType = 0;

		U32 hashValue = 0;

		U32 componentTypeHashValue = 0;

		E_RESULT_CODE result = RC_OK;

		for (auto& currEntityComponentPair : entityComponentsList)
		{
			componentType = currEntityComponentPair.first;
			hashValue = currEntityComponentPair.second - 1;

			componentTypeHashValue = mComponentsHashTable[componentType];

			pCurrComponent = mActiveComponents[componentTypeHashValue][hashValue];

			if ((result = action(pCurrComponent)) != RC_OK)
			{
				return result;
			}

			mActiveComponents[componentTypeHashValue][hashValue] = nullptr;
			
			// mark handlers as invalid
			mEntityComponentMap[entityId][componentType] = 0;
			mComponentEntityMap[componentType][entityId] = 0;
		}

		return RC_OK;
	}
	
	E_RESULT_CODE CComponentManager::RegisterFactory(const IComponentFactory* pFactory)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pFactory)
		{
			return RC_INVALID_ARGS;
		}

		TypeId componentTypeId = pFactory->GetComponentTypeId();

		U32 hashValue = 0;

		if (mFreeComponentFactoriesRegistry.empty())
		{
			hashValue = mComponentFactories.size();

			mComponentFactories.push_back(pFactory);

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

		//pNewComponent = pComponentFactory->Create(std::make_unique);  /// \todo implement arguments passing
		pNewComponent = pComponentFactory->CreateDefault({});
		
		auto componentTypeHashIter = mComponentsHashTable.find(componentTypeId);

		if (componentTypeHashIter == mComponentsHashTable.end()) /// allocate space for a new group
		{
			mComponentsHashTable[componentTypeId] = mActiveComponents.size();

			mActiveComponents.emplace_back();
		}
		
		std::vector<IComponent*>& componentsGroup = mActiveComponents[mComponentsHashTable[componentTypeId]];

		componentsGroup.push_back(pNewComponent);

		size_t hash = componentsGroup.size();

		mComponentEntityMap[componentTypeId][entityId] = hash;
		mEntityComponentMap[entityId][componentTypeId] = hash;

		return pNewComponent;
	}

	IComponent* CComponentManager::_getComponent(TypeId componentTypeId, TEntityId entityId)
	{
		U32 instanceHashValue = mComponentEntityMap[componentTypeId][entityId];
		
		if (!instanceHashValue || (mComponentsHashTable.find(componentTypeId) == mComponentsHashTable.cend()))
		{
			return nullptr;
		}

		U32 componentTypeHashValue = mComponentsHashTable[componentTypeId];

		return mActiveComponents[componentTypeHashValue][instanceHashValue - 1];
	}

	E_RESULT_CODE CComponentManager::_registerBuiltinComponentFactories()
	{
		E_RESULT_CODE result = RC_OK;

		auto builtinComponentFactories =
		{
			CreateTransformFactory,
			CreateQuadSpriteFactory,
			CreatePerspectiveCameraFactory,
			CreateOrthoCameraFactory
			//etc
		};

		IComponentFactory* pCurrFactory = nullptr;

		for (auto pCurrFactoryCallback : builtinComponentFactories)
		{
			IComponentFactory* pCurrFactory = pCurrFactoryCallback(result);

			if (result != RC_OK)
			{
				return result;
			}

			if ((result = RegisterFactory(pCurrFactory)) != RC_OK)
			{
				return result;
			}
		}

		return RC_OK;
	}

	E_RESULT_CODE CComponentManager::_unregisterBuiltinComponentFactories()
	{
		E_RESULT_CODE result = RC_OK;

		TypeId builtinComponentTypes[] =
		{
			CTransform::GetTypeId(),
			CQuadSprite::GetTypeId(),
			CPerspectiveCamera::GetTypeId(),
			COrthoCamera::GetTypeId()
			//etc
		};

		IComponentFactory* pCurrFactory = nullptr;

		for (TypeId currComponentType : builtinComponentTypes)
		{
			result = _unregisterFactory(currComponentType);

			if (result != RC_OK)
			{
				return result;
			}
		}

		return RC_OK;
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