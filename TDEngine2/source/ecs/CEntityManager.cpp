#include "./../../include/ecs/CEntityManager.h"
#include "./../../include/ecs/CEntity.h"
#include "./../../include/ecs/CTransform.h"


namespace TDEngine2
{
	CEntityManager::CEntityManager() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CEntityManager::Init(IComponentManager* pComponentManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pComponentManager)
		{
			return RC_INVALID_ARGS;
		}

		mpComponentManager = pComponentManager;

		mNextIdValue = 0;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CEntityManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = DestroyAllImmediately();

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	CEntity* CEntityManager::Create()
	{
		E_RESULT_CODE result = RC_OK;

		CEntity* pEntity = CreateEntity(mNextIdValue, _constructDefaultEntityName(mNextIdValue), this, result);

		if (result != RC_OK)
		{
			return nullptr;
		}

		size_t currStorageSize = mActiveEntities.size();

		if (mNextIdValue + 1 >= currStorageSize)
		{
			mActiveEntities.resize(currStorageSize + 10); /// \todo 10 is just a magic constant that should be replaced in some way
		}
		
		mActiveEntities[mNextIdValue++] = pEntity;

		/// create basic component CTransform
		pEntity->AddComponent<CTransform>();
		
		return pEntity;
	}

	CEntity* CEntityManager::Create(const std::string& name)
	{
		E_RESULT_CODE result = RC_OK;

		CEntity* pEntity = CreateEntity(mNextIdValue, name, this, result);

		if (result != RC_OK)
		{
			return nullptr;
		}

		size_t currStorageSize = mActiveEntities.size();

		if (mNextIdValue + 1 >= currStorageSize)
		{
			mActiveEntities.resize(currStorageSize + 10); /// \todo 10 is just a magic constant that should be replaced in some way
		}

		mActiveEntities[mNextIdValue++] = pEntity;

		/// create basic component CTransform
		pEntity->AddComponent<CTransform>();

		return pEntity;
	}

	E_RESULT_CODE CEntityManager::Destroy(CEntity* pEntity)
	{
		if (!pEntity)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = pEntity->RemoveComponents();

		if (result != RC_OK)
		{
			return result;
		}

		mActiveEntities[pEntity->GetId()] = nullptr;

		mDestroyedEntities.push_back(pEntity);

		return RC_OK;
	}

	E_RESULT_CODE CEntityManager::DestroyImmediately(CEntity* pEntity)
	{
		if (!pEntity)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = pEntity->RemoveComponents();

		if (result != RC_OK)
		{
			return result;
		}

		result = pEntity->Free();

		if (result != RC_OK)
		{
			return result;
		}

		mActiveEntities[pEntity->GetId()] = nullptr;
		
		return RC_OK;
	}

	E_RESULT_CODE CEntityManager::DestroyAllEntities()
	{
		E_RESULT_CODE result = RC_OK;

		for (CEntity* pEntity : mActiveEntities)
		{
			if ((result = Destroy(pEntity)) != RC_OK)
			{
				return result;
			}
		}

		return RC_OK;
	}

	E_RESULT_CODE CEntityManager::DestroyAllImmediately()
	{
		E_RESULT_CODE result = RC_OK;

		for (CEntity* pEntity : mActiveEntities)
		{
			if ((result = DestroyImmediately(pEntity)) != RC_OK)
			{
				return result;
			}
		}

		return RC_OK;
	}

	E_RESULT_CODE CEntityManager::RemoveComponents(TEntityId id)
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	std::string CEntityManager::_constructDefaultEntityName(U32 id) const
	{
		return std::move(std::string("Entity").append(std::to_string(id)));
	}
	

	CEntityManager* CreateEntityManager(IComponentManager* pComponentManager, E_RESULT_CODE& result)
	{
		CEntityManager* pEntityManager = new (std::nothrow) CEntityManager();

		if (!pEntityManager)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pEntityManager->Init(pComponentManager);

		if (result != RC_OK)
		{
			delete pEntityManager;

			pEntityManager = nullptr;
		}

		return pEntityManager;
	}
}