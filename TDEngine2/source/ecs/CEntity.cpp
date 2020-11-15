#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CEntityManager.h"
#include <memory>


namespace TDEngine2
{
	CEntity::CEntity():
		CBaseObject()
	{
	}

	E_RESULT_CODE CEntity::Init(TEntityId id, const std::string& name, CEntityManager* pEntityManager)
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

		mName = name;

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

	E_RESULT_CODE CEntity::Load(IArchiveReader* pReader)
	{
		TDE2_UNIMPLEMENTED();
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CEntity::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("entity");
		{
			pWriter->SetUInt32("id", static_cast<U32>(mId));
			pWriter->SetString("name", mName);

			pWriter->BeginGroup("components", true);
			{
				auto&& components = mpEntityManager->GetComponents(mId);
				for (auto&& currComponent : components)
				{
					if (!currComponent)
					{
						continue;
					}

					pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
					{
						currComponent->Save(pWriter);
					}
					pWriter->EndGroup();
				}
			}
			pWriter->EndGroup();
		}
		pWriter->EndGroup();

		return RC_OK;
	}
	
	void CEntity::SetId(TEntityId id)
	{
		mId = id;
	}

	void CEntity::SetName(const std::string& name)
	{
		mName = name;
	}

	void CEntity::Reset()
	{
		mId = TEntityId::Invalid;
	}

	E_RESULT_CODE CEntity::RemoveComponents()
	{
		return mpEntityManager->RemoveComponents(mId);
	}

	TEntityId CEntity::GetId() const
	{
		return mId;
	}

	const std::string& CEntity::GetName() const
	{
		return mName;
	}

	bool CEntity::operator== (const CEntity* pEntity) const
	{
		return pEntity && (mId == pEntity->mId);
	}

	bool CEntity::operator!= (const CEntity* pEntity) const
	{
		return pEntity && (mId != pEntity->mId);
	}

	bool CEntity::operator== (TEntityId otherId) const
	{
		return mId == otherId;
	}

	bool CEntity::operator!= (TEntityId otherId) const
	{
		return mId != otherId;
	}


	CEntity* CreateEntity(TEntityId id, const std::string& name, CEntityManager* pEntityManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CEntity, CEntity, result, id, name, pEntityManager);
	}
}