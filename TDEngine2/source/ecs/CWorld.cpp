#include "./../../include/ecs/CWorld.h"
#include "./../../include/ecs/CEntity.h"
#include "./../../include/ecs/ISystemManager.h"
#include "./../../include/ecs/CEntityManager.h"
#include "./../../include/ecs/CComponentManager.h"
#include "./../../include/ecs/CSystemManager.h"
#include "./../../include/core/IEventManager.h"
#include "./../../include/editor/CPerfProfiler.h"
#include "./../../include/physics/IRaycastContext.h"


namespace TDEngine2
{
	CWorld::CWorld():
		CBaseObject(), mpEventManager(nullptr)
	{
	}

	E_RESULT_CODE CWorld::Init(IEventManager* pEventManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pEventManager)
		{
			return RC_INVALID_ARGS;
		}
		
		E_RESULT_CODE result = RC_OK;
		
		mpComponentManager = CreateComponentManager(result);

		if (result != RC_OK)
		{
			return result;
		}

		mpEntityManager = CreateEntityManager(pEventManager, mpComponentManager, result);

		if (result != RC_OK)
		{
			return result;
		}
		
		mpSystemManager = CreateSystemManager(this, pEventManager, result);

		if (result != RC_OK)
		{
			return result;
		}

		mpEventManager   = pEventManager;
		mpRaycastContext = nullptr;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CWorld::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

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

		mIsInitialized = false;

		delete this;

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

	E_RESULT_CODE CWorld::RegisterRaycastContext(IRaycastContext* pRaycastContext)
	{
		if (!pRaycastContext)
		{
			return RC_INVALID_ARGS;
		}

		mpRaycastContext = pRaycastContext;

		return RC_OK;
	}

	CEntity* CWorld::FindEntity(TEntityId entityId) const
	{
		return mpEntityManager->GetEntity(entityId);
	}
	
	void CWorld::Update(float dt)
	{
		TDE2_PROFILER_SCOPE("World::Update");
		mpSystemManager->Update(this, dt);

		// \note reset all allocated raycasts results data
		mpRaycastContext->Reset();
	}

	IRaycastContext* CWorld::GetRaycastContext() const
	{
		return mpRaycastContext;
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
	

	IWorld* CreateWorld(IEventManager* pEventManager, E_RESULT_CODE& result)
	{
		CWorld* pWorld = new (std::nothrow) CWorld();

		if (!pWorld)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pWorld->Init(pEventManager);

		if (result != RC_OK)
		{
			delete pWorld;

			pWorld = nullptr;
		}

		return dynamic_cast<IWorld*>(pWorld);
	}
}