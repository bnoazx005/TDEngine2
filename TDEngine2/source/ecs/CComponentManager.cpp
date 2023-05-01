#include "../../include/ecs/CComponentManager.h"
#include "../../include/ecs/IComponentFactory.h"
#include "../../include/ecs/IComponent.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/ecs/CBaseComponent.h"
#include "../../include/ecs/components/CBoundsComponent.h"
#include "../../include/graphics/CQuadSprite.h"
#include "../../include/graphics/CPerspectiveCamera.h"
#include "../../include/graphics/COrthoCamera.h"
#include "../../include/graphics/CStaticMeshContainer.h"
#include "../../include/graphics/CSkinnedMeshContainer.h"
#include "../../include/graphics/effects/CParticleEmitterComponent.h"
#include "../../include/graphics/UI/CLayoutElementComponent.h"
#include "../../include/graphics/UI/CCanvasComponent.h"
#include "../../include/graphics/UI/CUIElementMeshDataComponent.h"
#include "../../include/graphics/UI/CImageComponent.h"
#include "../../include/graphics/UI/CInputReceiverComponent.h"
#include "../../include/graphics/UI/CLabelComponent.h"
#include "../../include/graphics/UI/C9SliceImageComponent.h"
#include "../../include/graphics/UI/GroupLayoutComponents.h"
#include "../../include/graphics/UI/CToggleComponent.h"
#include "../../include/graphics/UI/CUISliderComponent.h"
#include "../../include/graphics/UI/CInputFieldComponent.h"
#include "../../include/graphics/UI/CScrollableUIAreaComponent.h"
#include "../../include/graphics/animation/CAnimationContainerComponent.h"
#include "../../include/graphics/animation/CMeshAnimatorComponent.h"
#include "../../include/physics/2D/CBoxCollisionObject2D.h"
#include "../../include/physics/2D/CCircleCollisionObject2D.h"
#include "../../include/physics/2D/CTrigger2D.h"
#include "../../include/scene/components/CDirectionalLight.h"
#include "../../include/scene/components/CPointLight.h"
#include "../../include/scene/components/ShadowMappingComponents.h"
#include "../../include/scene/components/AudioComponents.h"
#include "../../include/scene/components/CLODStrategyComponent.h"
#include "../../include/scene/components/CObjIdComponent.h"
#include "../../include/editor/ecs/EditorComponents.h"
#include "../../include/editor/CPerfProfiler.h"


namespace TDEngine2
{
	std::unordered_set<TypeId> CComponentManager::mUniqueComponentTypesRegistry;

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

	E_RESULT_CODE CComponentManager::_onFreeInternal()
	{
		E_RESULT_CODE result = RC_OK;

		/// \note Remove all active components
		for (auto&& currComponentsGroup : mActiveComponents)
		{
			for (IComponent* pCurrComponent : currComponentsGroup)
			{
				if (!pCurrComponent)
				{
					continue;
				}

				result = result | pCurrComponent->Free();
			}
		}

		/// \note Remove reserved ones
		for (IComponent* pReservedComponent : mDestroyedComponents)
		{
			if (!pReservedComponent)
			{
				continue;
			}

			pReservedComponent->Free();
		}

		mActiveComponents.clear();
		mDestroyedComponents.clear();

		result = result | _unregisterBuiltinComponentFactories();

		return result;
	}

	IComponent* CComponentManager::CreateComponent(TEntityId entityId, TypeId componentTypeId)
	{
		return _createComponent(componentTypeId, entityId);
	}

	E_RESULT_CODE CComponentManager::RemoveComponent(TypeId componentTypeId, TEntityId entityId)
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
		
		if (_isUniqueComponent(componentTypeId))
		{
			auto it = mUniqueComponentsRegistry.find(componentTypeId);
			if (it != mUniqueComponentsRegistry.cend())
			{
				mUniqueComponentsRegistry.erase(it);
			}
		}

		// mark handlers as invalid
		mEntityComponentMap[entityId].erase(componentTypeId);
		mComponentEntityMap[componentTypeId].erase(entityId);

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

			if (_isUniqueComponent(componentType))
			{
				auto it = mUniqueComponentsRegistry.find(componentType);
				if (it != mUniqueComponentsRegistry.cend())
				{
					mUniqueComponentsRegistry.erase(it);
				}
			}

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

	bool CComponentManager::HasComponent(TEntityId id, TypeId componentTypeId)
	{
		return _hasComponent(componentTypeId, id);
	}

	E_RESULT_CODE CComponentManager::RegisterFactory(TPtr<IComponentFactory> pFactory)
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

#if TDE2_EDITORS_ENABLED
		mRegisteredComponentTypes.push_back({ pFactory->GetComponentTypeStr(), componentTypeId });
#endif

		if (mFreeComponentFactoriesRegistry.empty())
		{
			hashValue = static_cast<U32>(mComponentFactories.size());

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

	E_RESULT_CODE CComponentManager::UnregisterFactory(TypeId typeId)
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

#if TDE2_EDITORS_ENABLED
		auto it = std::find_if(mRegisteredComponentTypes.cbegin(), mRegisteredComponentTypes.cend(), [name = mComponentFactories[hashValue]->GetComponentTypeStr()](auto&& t)
		{
			return t.mName == name;
		});

		if (it != mRegisteredComponentTypes.cend())
		{
			mRegisteredComponentTypes.erase(it);
		}
#endif

		mComponentFactories[hashValue] = nullptr;

		mFreeComponentFactoriesRegistry.push_front(hashValue);

		return RC_OK;
	}

	IComponent* CComponentManager::_createComponent(TypeId componentTypeId, TEntityId entityId)
	{
		if (TEntityId::Invalid == entityId)
		{
			return nullptr;
		}

		IComponent* pNewComponent = _getComponent(componentTypeId, entityId);

		if (pNewComponent)
		{
			return pNewComponent;
		}

		/// \note If there is already an entity with the corresponding component
		const bool isUniqueComponentType = _isUniqueComponent(componentTypeId);

		if (isUniqueComponentType && TEntityId::Invalid != FindEntityWithUniqueComponent(componentTypeId))
		{
			TDE2_UNREACHABLE();
			return nullptr;
		}

		TComponentFactoriesMap::const_iterator factoryIdIter = mComponentFactoriesMap.find(componentTypeId);

		if (factoryIdIter == mComponentFactoriesMap.cend())
		{
			return nullptr;
		}

		auto pComponentFactory = mComponentFactories[(*factoryIdIter).second];

		pNewComponent = pComponentFactory->CreateDefault();
		
		auto componentTypeHashIter = mComponentsHashTable.find(componentTypeId);

		if (componentTypeHashIter == mComponentsHashTable.end()) /// allocate space for a new group
		{
			mComponentsHashTable[componentTypeId] = static_cast<U32>(mActiveComponents.size());

			mActiveComponents.emplace_back();
		}
		
		std::vector<IComponent*>& componentsGroup = mActiveComponents[mComponentsHashTable[componentTypeId]];

		componentsGroup.push_back(pNewComponent);

		U32 hash = static_cast<U32>(componentsGroup.size());

		mComponentEntityMap[componentTypeId][entityId] = hash;
		mEntityComponentMap[entityId][componentTypeId] = hash;

		if (isUniqueComponentType)
		{
			mUniqueComponentsRegistry[componentTypeId] = entityId;
		}

		return pNewComponent;
	}

	IComponent* CComponentManager::_getComponent(TypeId componentTypeId, TEntityId entityId)
	{
		TDE2_PROFILER_SCOPE("CComponentManager::_getComponent");
		
		const U32 instanceHashValue = mComponentEntityMap[componentTypeId][entityId];

		auto&& it = mComponentsHashTable.find(componentTypeId);

		if (!instanceHashValue || (it == mComponentsHashTable.cend()))
		{
			return nullptr;
		}		

		return mActiveComponents[it->second][instanceHashValue - 1];
	}

	E_RESULT_CODE CComponentManager::_registerBuiltinComponentFactories()
	{
		E_RESULT_CODE result = RC_OK;

		auto builtinComponentFactories =
		{
			CreateTransformFactory,
			CreateDeactivatedComponentFactory,
			CreateDeactivatedGroupComponentFactory,
			CreateQuadSpriteFactory,
			CreatePerspectiveCameraFactory,
			CreateOrthoCameraFactory,
			CreateCamerasContextComponentFactory,
			CreateBoxCollisionObject2DFactory,
			CreateCircleCollisionObject2DFactory,
			CreateTrigger2DFactory,
			CreateMeshAnimatorComponentFactory,
			CreateStaticMeshContainerFactory,
			CreateSkinnedMeshContainerFactory,
			CreateBoundsComponentFactory,
			CreateDirectionalLightFactory,
			CreatePointLightFactory,
			CreateShadowCasterComponentFactory,
			CreateShadowReceiverComponentFactory,
			CreateAudioListenerComponentFactory,
			CreateAudioSourceComponentFactory,
			CreateAnimationContainerComponentFactory,
			CreateSkyboxComponentFactory,
			CreateParticleEmitterFactory,
			CreateLayoutElementFactory,
			CreateCanvasFactory,
			CreateUIElementMeshDataFactory,
			CreateImageFactory,
			CreateInputReceiverFactory,
			CreateLabelFactory,
			Create9SliceImageFactory,
			CreateGridGroupLayoutFactory,
			CreateToggleFactory,
			CreateUISliderFactory,
			CreateInputFieldFactory,
			CreateUIMaskComponentFactory,
			CreateScrollableUIAreaFactory,
			CreateLODStrategyComponentFactory,
			CreateObjIdComponentFactory,
#if TDE2_EDITORS_ENABLED
			CreateSelectedEntityComponentFactory,
			CreateSceneInfoComponentFactory,
			CreatePrefabLinkInfoComponentFactory,
			CreateEditorCameraFactory,
#endif
			//etc
		};

		for (auto pCurrFactoryCallback : builtinComponentFactories)
		{
			TPtr<IComponentFactory> pCurrFactory = TPtr<IComponentFactory>(pCurrFactoryCallback(result));

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

		for (auto&& pCurrFactory : mComponentFactories)
		{
			if (!pCurrFactory)
			{
				continue;
			}

			result = result | UnregisterFactory(pCurrFactory->GetComponentTypeId());
		}

		return result;
	}

	bool CComponentManager::_hasComponent(TypeId componentTypeId, TEntityId entityId)
	{
		TDE2_PROFILER_SCOPE("CComponentManager::_hasComponent");

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

	void CComponentManager::ForEachFactory(const std::function<void(TPtr<IComponentFactory>)>& action)
	{
		if (!action)
		{
			return;
		}

		for (TPtr<IComponentFactory> pCurrFactory : mComponentFactories)
		{
			action(pCurrFactory);
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

			if (_hasComponent(CDeactivatedComponent::GetTypeId(), entityComponentsTablePair.first) || 
				_hasComponent(CDeactivatedGroupComponent::GetTypeId(), entityComponentsTablePair.first))
			{
				continue; /// If the entity marked with CDeactivatedComponent component it means that is should be skipped from processing
			}

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

			if (_hasComponent(CDeactivatedComponent::GetTypeId(), entityComponentsTablePair.first) ||
				_hasComponent(CDeactivatedGroupComponent::GetTypeId(), entityComponentsTablePair.first))
			{
				continue; /// If the entity marked with CDeactivatedComponent component it means that is should be skipped from processing
			}

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

	TEntityId CComponentManager::FindEntityWithUniqueComponent(TypeId typeId)
	{
		if (mUniqueComponentsRegistry.empty())
		{
			return TEntityId::Invalid;
		}

		auto it = mUniqueComponentsRegistry.find(typeId);
		return (it == mUniqueComponentsRegistry.cend()) ? TEntityId::Invalid : it->second;
	}

	E_RESULT_CODE CComponentManager::RegisterUniqueComponentType(TypeId typeId)
	{
		if (mUniqueComponentTypesRegistry.find(typeId) != mUniqueComponentTypesRegistry.cend())
		{
			return RC_FAIL;
		}

		mUniqueComponentTypesRegistry.emplace(typeId);

		return RC_OK;
	}

#if TDE2_EDITORS_ENABLED

	const std::vector<TComponentTypeInfo>& CComponentManager::GetRegisteredComponentsIdentifiers() const
	{
		return mRegisteredComponentTypes;
	}

#endif

	bool CComponentManager::_isUniqueComponent(TypeId componentTypeId) const
	{
		return mUniqueComponentTypesRegistry.find(componentTypeId) != mUniqueComponentTypesRegistry.cend();
	}

	IComponentManager* CreateComponentManager(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentManager, CComponentManager, result);
	}
}