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

	E_RESULT_CODE CEntity::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		pReader->BeginGroup("entity");
		{
			mName = pReader->GetString("name");

			pReader->BeginGroup("components");
			{
				while (pReader->HasNextItem())
				{
					pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
					{
						pReader->BeginGroup("component");
						{
							IComponent* pComponent = _addComponentInternal(static_cast<TypeId>(pReader->GetUInt32("type_id"))); // \note Construct a new component based on its type
							TDE2_ASSERT(pComponent);

							if (pComponent)
							{
								pComponent->Load(pReader);
							}
						}
						pReader->EndGroup();
					}
					pReader->EndGroup();
				}
			}
			pReader->EndGroup();
		}
		pReader->EndGroup();

		return RC_OK;
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

	E_RESULT_CODE CEntity::PostLoad(CEntityManager* pEntityManager, const std::unordered_map<TEntityId, TEntityId>& entitiesIdentifiersRemapper)
	{
		E_RESULT_CODE result = RC_OK;

		for (IComponent* pComponent : GetComponents())
		{
			result = result | pComponent->PostLoad(pEntityManager, entitiesIdentifiersRemapper);
		}

		return result;
	}

	E_RESULT_CODE CEntity::Clone(CEntity*& pDestObject) const
	{
		E_RESULT_CODE result = RC_OK;

		const TEntityId destEntityId = pDestObject->GetId();

		for (IComponent* pComponent : GetComponents())
		{
			if (auto pComponentCopy = pDestObject->AddComponent(pComponent->GetComponentTypeId()))
			{
				pComponent->Clone(pComponentCopy);
			}
		}

		pDestObject->SetName(mName);

		return result;
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

	IComponent* CEntity::AddComponent(TypeId componentTypeId)
	{
		return mpEntityManager->AddComponent(mId, componentTypeId);
	}

	std::vector<IComponent*> CEntity::GetComponents() const
	{
		return mpEntityManager->GetComponents(mId);
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

	IComponent* CEntity::_addComponentInternal(TypeId typeId)
	{
		return mpEntityManager->AddComponent(mId, typeId);
	}


	CEntity* CreateEntity(TEntityId id, const std::string& name, CEntityManager* pEntityManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CEntity, CEntity, result, id, name, pEntityManager);
	}
}