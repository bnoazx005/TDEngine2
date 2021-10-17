#include "../../include/ecs/CWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/ISystemManager.h"
#include "../../include/ecs/CEntityManager.h"
#include "../../include/ecs/CComponentManager.h"
#include "../../include/ecs/CSystemManager.h"
#include "../../include/core/IEventManager.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/physics/IRaycastContext.h"
#include "../../include/ecs/CTransform.h"


namespace TDEngine2
{
	CWorld::CWorld():
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

		mpEventManager   = pEventManager;
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

	E_RESULT_CODE CWorld::DestroyImmediately(CEntity* pEntity)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mpEntityManager->DestroyImmediately(pEntity);
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

	E_RESULT_CODE CWorld::RegisterRaycastContext(IRaycastContext* pRaycastContext)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!pRaycastContext)
		{
			return RC_INVALID_ARGS;
		}

		mpRaycastContext = pRaycastContext;

		return RC_OK;
	}

	CEntity* CWorld::FindEntity(TEntityId entityId) const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mpEntityManager->GetEntity(entityId);
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

	IRaycastContext* CWorld::GetRaycastContext() const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mpRaycastContext;
	}

	F32 CWorld::GetTimeScaleFactor() const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mTimeScaleFactor;
	}

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
			return RC_OK;
		}

		CEntity* pParentEntity = pWorld->FindEntity(parentEntity);
		if (!pParentEntity)
		{
			return RC_FAIL;
		}

		pParentEntity->GetComponent<CTransform>()->AttachChild(childEntity);

		return RC_OK;
	}
}