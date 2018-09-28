#include "./../../include/ecs/CEntity.h"
#include "./../../include/ecs/CEntityManager.h"
#include <memory>


namespace TDEngine2
{
	CEntity::CEntity():
		CBaseObject()
	{
	}

	E_RESULT_CODE CEntity::Init(TEntityId id, CEntityManager* pEntityManager)
	{
		if (!pEntityManager)
		{
			return RC_INVALID_ARGS;
		}

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mpEntityManager = pEntityManager;

		mId = id;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CEntity::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}
	
	void CEntity::SetId(TEntityId id)
	{
		mId = id;
	}

	void CEntity::Reset()
	{
		mId = InvalidEntityId;
	}

	E_RESULT_CODE CEntity::RemoveComponents()
	{
		return mpEntityManager->RemoveComponents(mId);
	}

	TEntityId CEntity::GetId() const
	{
		return mId;
	}


	CEntity* CreateEntity(TEntityId id, CEntityManager* pEntityManager, E_RESULT_CODE& result)
	{
		CEntity* pEntity = new (std::nothrow) CEntity();

		if (!pEntity)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pEntity->Init(id, pEntityManager);

		return pEntity;
	}
}