#include "./../../include/ecs/CWorld.h"
#include "./../../include/ecs/CEntity.h"
#include "./../../include/ecs/ISystemManager.h"
#include "./../../include/ecs/CEntityManager.h"
#include "./../../include/ecs/CComponentManager.h"
#include "./../../include/ecs/CSystemManager.h"
#include "./../../include/core/IEventManager.h"


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

		mpEventManager = pEventManager;

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
	
	E_RESULT_CODE CWorld::RegisterSystem(ISystem* pSystem)
	{
		return mpSystemManager->RegisterSystem(pSystem);
	}
		
	E_RESULT_CODE CWorld::UnregisterSystem(ISystem* pSystem)
	{
		return mpSystemManager->UnregisterSystem(pSystem);
	}
	
	E_RESULT_CODE CWorld::UnregisterSystemImmediately(ISystem* pSystem)
	{
		return mpSystemManager->UnregisterSystemImmediately(pSystem);
	}
	
	E_RESULT_CODE CWorld::ActivateSystem(ISystem* pSystem)
	{
		return mpSystemManager->ActivateSystem(pSystem);
	}
	
	E_RESULT_CODE CWorld::DeactivateSystem(ISystem* pSystem)
	{
		return mpSystemManager->DeactivateSystem(pSystem);
	}

	CEntity* CWorld::FindEntity(TEntityId entityId) const
	{
		return mpEntityManager->GetEntity(entityId);
	}
	
	void CWorld::Update(float dt)
	{
		mpSystemManager->Update(this, dt);
	}

	CComponentIterator CWorld::_findComponentsOfType(TypeId typeId)
	{
		return mpComponentManager->FindComponentsOfType(typeId);
	}

	void CWorld::_forEach(TComponentTypeId componentTypeId, const std::function<void(TEntityId entityId, IComponent* pComponent)>& action)
	{
		mpComponentManager->ForEach(componentTypeId, action);
	}

	std::vector<TEntityId> CWorld::_findEntitiesWithComponents(const std::vector<TComponentTypeId>& types)
	{
		return mpComponentManager->FindEntitiesWithComponents(types);
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