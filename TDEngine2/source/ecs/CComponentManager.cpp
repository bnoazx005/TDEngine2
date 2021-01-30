#include "../../include/ecs/CComponentManager.h"
#include "../../include/ecs/IComponentFactory.h"
#include "../../include/ecs/IComponent.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/ecs/CBaseComponent.h"
#include "../../include/graphics/CQuadSprite.h"
#include "../../include/graphics/CPerspectiveCamera.h"
#include "../../include/graphics/COrthoCamera.h"
#include "../../include/graphics/CStaticMeshContainer.h"
#include "../../include/physics/2D/CBoxCollisionObject2D.h"
#include "../../include/physics/2D/CCircleCollisionObject2D.h"
#include "../../include/physics/2D/CTrigger2D.h"
#include "../../include/physics/3D/CBoxCollisionObject3D.h"
#include "../../include/physics/3D/CSphereCollisionObject3D.h"
#include "../../include/physics/3D/CConvexHullCollisionObject3D.h"
#include "../../include/physics/3D/CTrigger3D.h"
#include "../../include/scene/components/CDirectionalLight.h"
#include "../../include/scene/components/ShadowMappingComponents.h"
#include "../../include/scene/components/AudioComponents.h"
#include "../../include/ecs/components/CBoundsComponent.h"
#include "../../include/graphics/animation/CAnimationContainerComponent.h"
#include "../../include/editor/ecs/EditorComponents.h"


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

	IComponent* CComponentManager::CreateComponent(TEntityId entityId, TypeId componentTypeId)
	{
		return _createComponent(componentTypeId, entityId);
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

		U32 componentHashValue = mComponentsHashTable[componentTypeId];

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

		mActiveComponents[componentHashValue][targetEntityComponentHash - 1] = nullptr;
		
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

		TypeId componentType = TypeId(0);

		U32 hashValue = 0;

		U32 componentTypeHashValue = 0;

		E_RESULT_CODE result = RC_OK;

		for (auto& currEntityComponentPair : entityComponentsList)
		{
			componentType = currEntityComponentPair.first;
			hashValue = currEntityComponentPair.second - 1;

			if (hashValue == (std::numeric_limits<U32>::max)()) // \note this means that there is no component of given type on this entity
			{
				continue;
			}

			componentTypeHashValue = mComponentsHashTable[componentType];

			pCurrComponent = mActiveComponents[componentTypeHashValue][hashValue];

			if ((result = action(pCurrComponent)) != RC_OK)
			{
				return result;
			}

			mActiveComponents[componentTypeHashValue][hashValue] = nullptr;
			
			// mark handlers as invalid
			mEntityComponentMap[entityId].erase(componentType);
			mComponentEntityMap[componentType].erase(entityId);
		}

		return RC_OK;
	}
	
	std::vector<IComponent*> CComponentManager::GetComponents(TEntityId id) const
	{
		std::vector<IComponent*> components;

		auto entityComponentsIter = mEntityComponentMap.find(id);
		if (entityComponentsIter == mEntityComponentMap.cend())
		{
			return components;
		}

		auto&& componentsHashesTable = entityComponentsIter->second;

		for (auto& currEntityComponentPair : componentsHashesTable)
		{
			const TypeId componentType = currEntityComponentPair.first;

			 components.push_back(mActiveComponents[mComponentsHashTable.at(componentType)][currEntityComponentPair.second - 1]);
		}

		return components;
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
		if (typeId == TypeId::Invalid)
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
			CreateOrthoCameraFactory,
			CreateBoxCollisionObject2DFactory,
			CreateCircleCollisionObject2DFactory,
			CreateTrigger2DFactory,
			CreateStaticMeshContainerFactory,
			CreateBoxCollisionObject3DFactory,
			CreateSphereCollisionObject3DFactory,
			CreateConvexHullCollisionObject3DFactory,
			CreateTrigger3DFactory,
			CreateBoundsComponentFactory,
			CreateDirectionalLightFactory,
			CreateShadowCasterComponentFactory,
			CreateShadowReceiverComponentFactory,
			CreateAudioListenerComponentFactory,
			CreateAudioSourceComponentFactory,
			CreateAnimationContainerComponentFactory,
#if TDE2_EDITORS_ENABLED
			CreateSelectedEntityComponentFactory,
			CreateSceneInfoComponentFactory,
#endif
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
			COrthoCamera::GetTypeId(),
			CBoxCollisionObject2D::GetTypeId(),
			CCircleCollisionObject2D::GetTypeId(),
			CTrigger2D::GetTypeId(),
			CStaticMeshContainer::GetTypeId(),
			CBoxCollisionObject3D::GetTypeId(),
			CSphereCollisionObject3D::GetTypeId(),
			CConvexHullCollisionObject3D::GetTypeId(),
			CTrigger3D::GetTypeId(),
			CBoundsComponent::GetTypeId(),
			CDirectionalLight::GetTypeId(),
			CShadowCasterComponent::GetTypeId(),
			CShadowReceiverComponent::GetTypeId(),
			CAudioListenerComponent::GetTypeId(),
			CAudioSourceComponent::GetTypeId(),
			CAnimationContainerComponent::GetTypeId(),
#if TDE2_EDITORS_ENABLED
			CSelectedEntityComponent::GetTypeId(),
			CSceneInfoComponent::GetTypeId(),
#endif
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

	bool CComponentManager::_hasComponent(TypeId componentTypeId, TEntityId entityId)
	{
		U32 instanceHashValue = mComponentEntityMap[componentTypeId][entityId];

		return instanceHashValue && (mComponentsHashTable.find(componentTypeId) != mComponentsHashTable.cend());
	}

	CComponentIterator CComponentManager::FindComponentsOfType(TypeId typeId)
	{
		if (mComponentsHashTable.find(typeId) == mComponentsHashTable.cend())
		{
			return CComponentIterator::mInvalidIterator;
		}

		U32 componentTypeHashValue = mComponentsHashTable[typeId];
		
		return CComponentIterator(mActiveComponents[componentTypeHashValue], 0);
	}

	void CComponentManager::ForEach(TypeId componentTypeId, const std::function<void(TEntityId entityId, IComponent* pComponent)>& action)
	{
		if (mComponentsHashTable.find(componentTypeId) == mComponentsHashTable.cend())
		{
			return;
		}

		U32 componentTypeHashValue = mComponentsHashTable[componentTypeId];

		auto& componentsGroup = mComponentEntityMap[componentTypeId];

		for (auto iter = componentsGroup.begin(); iter != componentsGroup.end(); ++iter)
		{
			action((*iter).first, mActiveComponents[componentTypeHashValue][(*iter).second - 1]);
		}
	}

	std::vector<TEntityId> CComponentManager::FindEntitiesWithAll(const std::vector<TypeId>& types)
	{
		std::vector<TEntityId> filter;

		std::unordered_map<TypeId, U32> entityComponentsTable;

		bool containsAll = false;

		for (auto entityComponentsTablePair : mEntityComponentMap)
		{
			entityComponentsTable = entityComponentsTablePair.second;

			containsAll = true;

			for (auto typeIter = types.cbegin(); typeIter != types.cend(); ++typeIter)
			{
				containsAll = containsAll && (entityComponentsTable.find(*typeIter) != entityComponentsTable.cend());

				if (!containsAll) /// an early interruption of the loop if one of operands is false
				{
					break;
				}
			}

			if (!containsAll)
			{
				continue;
			}

			filter.push_back(entityComponentsTablePair.first);
		}

		return std::move(filter);
	}

	std::vector<TEntityId> CComponentManager::FindEntitiesWithAny(const std::vector<TypeId>& types)
	{
		std::vector<TEntityId> filter;

		std::unordered_map<TypeId, U32> entityComponentsTable;

		for (auto entityComponentsTablePair : mEntityComponentMap)
		{
			entityComponentsTable = entityComponentsTablePair.second;
			
			for (auto typeIter = types.cbegin(); typeIter != types.cend(); ++typeIter)
			{
				if (entityComponentsTable.find(*typeIter) != entityComponentsTable.cend())
				{
					filter.push_back(entityComponentsTablePair.first);

					break;
				}
			}
		}

		return std::move(filter);
	}

	IComponentManager* CreateComponentManager(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentManager, CComponentManager, result);
	}
}