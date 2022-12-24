#include "../../include/ecs/CWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/ISystemManager.h"
#include "../../include/ecs/CEntityManager.h"
#include "../../include/ecs/ISystem.h"
#include "../../include/ecs/CComponentManager.h"
#include "../../include/ecs/CSystemManager.h"
#include "../../include/core/IEventManager.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/physics/IRaycastContext.h"
#include "../../include/graphics/CBaseCamera.h"
#include "../../include/graphics/CPerspectiveCamera.h"
#include "../../include/graphics/COrthoCamera.h"
#include "../../include/ecs/CTransform.h"
#if TDE2_EDITORS_ENABLED
#include "../../include/editor/ecs/EditorComponents.h"
#endif


namespace TDEngine2
{
	CWorld::CWorld() :
		CBaseObject(), mpEventManager(nullptr)
	{
	}

	E_RESULT_CODE CWorld::Init(TPtr<IEventManager> pEventManager)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pEventManager)
		{
			return RC_INVALID_ARGS;
		}

		mTimeScaleFactor = 1.0f;

		E_RESULT_CODE result = RC_OK;

		mpComponentManager = CreateComponentManager(result);

		if (result != RC_OK)
		{
			return result;
		}

		mpEntityManager = CreateEntityManager(pEventManager.Get(), mpComponentManager, result);

		if (result != RC_OK)
		{
			return result;
		}

		mpSystemManager = CreateSystemManager(this, pEventManager.Get(), result);

		if (result != RC_OK)
		{
			return result;
		}

		mpEventManager = pEventManager;
		mpRaycastContext = nullptr;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CWorld::_onFreeInternal()
	{
		E_RESULT_CODE result = RC_OK;

		{
			std::lock_guard<std::mutex> lock(mMutex);

			if ((result = mpEntityManager->Free()) != RC_OK)
			{
				return result;
			}

			if ((result = mpComponentManager->Free()) != RC_OK)
			{
				return result;
			}

			if ((result = mpSystemManager->Free()) != RC_OK)
			{
				return result;
			}
		}

		return RC_OK;
	}

	CEntity* CWorld::CreateEntity()
	{
		return mpEntityManager->Create();
	}

	CEntity* CWorld::CreateEntity(const std::string& name)
	{
		return mpEntityManager->Create(name);
	}

	E_RESULT_CODE CWorld::Destroy(CEntity* pEntity)
	{
		return mpEntityManager->Destroy(pEntity);
	}

	E_RESULT_CODE CWorld::RegisterComponentFactory(TPtr<IComponentFactory> pFactory)
	{
		if (!pFactory)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = mpComponentManager->RegisterFactory(pFactory);
		if (RC_OK != result)
		{
			return result;
		}

		TOnNewComponentFactoryRegistered event;
		event.mpFactory = pFactory;

		return mpEventManager->Notify(&event) | result;
	}

	E_RESULT_CODE CWorld::UnregisterComponentFactory(TypeId componentTypeId)
	{
		return mpComponentManager->UnregisterFactory(componentTypeId);
	}

	TResult<TSystemId> CWorld::RegisterSystem(ISystem* pSystem, E_SYSTEM_PRIORITY priority)
	{
		return mpSystemManager->RegisterSystem(pSystem, priority);
	}
		
	E_RESULT_CODE CWorld::UnregisterSystem(TSystemId systemId)
	{
		return mpSystemManager->UnregisterSystem(systemId);
	}
	
	E_RESULT_CODE CWorld::UnregisterSystemImmediately(TSystemId systemId)
	{
		return mpSystemManager->UnregisterSystemImmediately(systemId);
	}
	
	E_RESULT_CODE CWorld::ActivateSystem(TSystemId systemId)
	{
		return mpSystemManager->ActivateSystem(systemId);
	}
	
	E_RESULT_CODE CWorld::DeactivateSystem(TSystemId systemId)
	{
		return mpSystemManager->DeactivateSystem(systemId);
	}

	E_RESULT_CODE CWorld::OnBeforeFree()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mpSystemManager->ClearSystemsRegistry();
	}

	E_RESULT_CODE CWorld::RegisterRaycastContext(TPtr<IRaycastContext> pRaycastContext)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!pRaycastContext)
		{
			return RC_INVALID_ARGS;
		}

		mpRaycastContext = pRaycastContext;

		return RC_OK;
	}

	E_RESULT_CODE CWorld::NotifyOnHierarchyChanged(TEntityId parentEntityId, TEntityId childEntityId)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (TEntityId::Invalid == childEntityId)
		{
			return RC_INVALID_ARGS;
		}

		TOnHierarchyChangedEvent onHierarchyChangedEvent;
		onHierarchyChangedEvent.mParentEntityId = parentEntityId;
		onHierarchyChangedEvent.mChildEntityId = childEntityId;

		return mpEventManager->Notify(&onHierarchyChangedEvent);
	}

	CEntity* CWorld::FindEntity(TEntityId entityId) const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mpEntityManager->GetEntity(entityId);
	}

	void CWorld::ForEachSystem(const std::function<void(TSystemId, const ISystem* const)> action) const
	{
		mpSystemManager->ForEachSystem(action);
	}

	void CWorld::ForEachComponentFactory(const std::function<void(TPtr<IComponentFactory>)>& action)
	{
		mpComponentManager->ForEachFactory(action);
	}

	void CWorld::SetTimeScaleFactor(F32 scaleFactor)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mTimeScaleFactor = scaleFactor;
	}

	void CWorld::Update(F32 dt)
	{
		//FIXME std::lock_guard<std::mutex> lock(mMutex);

		TDE2_PROFILER_SCOPE("World::Update");
		mpSystemManager->Update(this, mTimeScaleFactor * dt);

		// \note reset all allocated raycasts results data
		mpRaycastContext->Reset();
	}

#if TDE2_EDITORS_ENABLED
	
	void CWorld::DebugOutput(IDebugUtility* pDebugUtility, F32 dt)
	{
		TDE2_PROFILER_SCOPE("World::DebugOutput");

		mpSystemManager->ForEachSystem([=](TSystemId currSystemHandle, const ISystem* pSystem)
		{
			pSystem->DebugOutput(pDebugUtility, dt);
		});
	}

#endif

	TPtr<IRaycastContext> CWorld::GetRaycastContext() const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mpRaycastContext;
	}

	CEntityManager* CWorld::GetEntityManager() const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mpEntityManager;
	}

	F32 CWorld::GetTimeScaleFactor() const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mTimeScaleFactor;
	}

#if TDE2_EDITORS_ENABLED

	const std::vector<TComponentTypeInfo>& CWorld::GetRegisteredComponentsIdentifiers() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mpComponentManager->GetRegisteredComponentsIdentifiers();
	}

#endif

	CComponentIterator CWorld::_findComponentsOfType(TypeId typeId)
	{
		return mpComponentManager->FindComponentsOfType(typeId);
	}

	void CWorld::_forEach(TypeId componentTypeId, const std::function<void(TEntityId entityId, IComponent* pComponent)>& action)
	{
		mpComponentManager->ForEach(componentTypeId, action);
	}

	std::vector<TEntityId> CWorld::_findEntitiesWithComponents(const std::vector<TypeId>& types)
	{
		return mpComponentManager->FindEntitiesWithAll(types);
	}

	std::vector<TEntityId> CWorld::_findEntitiesWithAnyComponents(const std::vector<TypeId>& types)
	{
		return mpComponentManager->FindEntitiesWithAny(types);
	}

	TEntityId CWorld::_findEntityWithUniqueComponent(TypeId typeId)
	{
		TEntityId entityId = mpComponentManager->FindEntityWithUniqueComponent(typeId);
		if (TEntityId::Invalid == entityId) /// \note Create a new instance because it doesn't exist yet
		{
			CEntity* pNewEntity = mpEntityManager->Create();
			if (!pNewEntity)
			{
				return TEntityId::Invalid;
			}

			entityId = pNewEntity->GetId();

			auto pComponent = mpComponentManager->CreateComponent(entityId, typeId);
			TDE2_ASSERT(pComponent);

			return entityId;
		}

		return entityId;
	}

	TSystemId CWorld::_findSystem(TypeId typeId)
	{
		return mpSystemManager->FindSystem(typeId);
	}
	

	IWorld* CreateWorld(TPtr<IEventManager> pEventManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IWorld, CWorld, result, pEventManager);
	}


	TDE2_API E_RESULT_CODE GroupEntities(IWorld* pWorld, TEntityId parentEntity, TEntityId childEntity)
	{
		if (!pWorld || TEntityId::Invalid == childEntity || parentEntity == childEntity)
		{
			return RC_INVALID_ARGS;
		}

		CEntity* pChildEntity = pWorld->FindEntity(childEntity);
		if (!pChildEntity)
		{
			return RC_FAIL;
		}

		CTransform* pChildTransform = pChildEntity->GetComponent<CTransform>();
		
		// \note Remove previous link if it exists
		if (TEntityId::Invalid != pChildTransform->GetParent())
		{
			if (auto pParentEntity = pWorld->FindEntity(pChildTransform->GetParent()))
			{
				pParentEntity->GetComponent<CTransform>()->DettachChild(childEntity);
			}
		}

		// \note Create a new link between child and parent
		pChildTransform->SetParent(parentEntity);

		if (TEntityId::Invalid == parentEntity)
		{
			return pWorld->NotifyOnHierarchyChanged(parentEntity, childEntity);
		}

		CEntity* pParentEntity = pWorld->FindEntity(parentEntity);
		if (!pParentEntity)
		{
			return RC_FAIL;
		}

		pParentEntity->GetComponent<CTransform>()->AttachChild(childEntity);

		return pWorld->NotifyOnHierarchyChanged(parentEntity, childEntity);
	}

	ICamera* GetCurrentActiveCamera(IWorld* pWorld)
	{
		if (CEntity* pCamerasContextEntity = pWorld->FindEntity(pWorld->FindEntityWithUniqueComponent<CCamerasContextComponent>()))
		{
			if (auto pCamerasContext = pCamerasContextEntity->GetComponent<CCamerasContextComponent>())
			{
				if (auto pCameraEntity = pWorld->FindEntity(pCamerasContext->GetActiveCameraEntityId()))
				{
					return GetValidPtrOrDefault<ICamera*>(pCameraEntity->GetComponent<CPerspectiveCamera>(), pCameraEntity->GetComponent<COrthoCamera>());
				}
			}
		}

		return nullptr;
	}

#if TDE2_EDITORS_ENABLED

	TEntityId GetPrefabInstanceRootEntityId(const TPtr<IWorld>& pWorld, TEntityId entityId)
	{
		TEntityId currEntityId = entityId;
		CEntity* pEntity = nullptr;

		CTransform* pTransform = nullptr;

		while ((pEntity = pWorld->FindEntity(currEntityId)))
		{
			if (pEntity->HasComponent<CPrefabLinkInfoComponent>())
			{
				return currEntityId;
			}

			pTransform = pEntity->GetComponent<CTransform>();
			currEntityId = pTransform->GetParent();
		}

		return TEntityId::Invalid;
	}

#endif
}