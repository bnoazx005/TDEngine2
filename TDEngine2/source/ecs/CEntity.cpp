#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CEntityManager.h"
#include "../../include/ecs/IWorld.h"
#include <memory>
#include "stringUtils.hpp"


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
				for (IComponent* pCurrComponent : components)
				{
					if (!pCurrComponent)
					{
						continue;
					}
					TDE2_ASSERT(pCurrComponent);

					if (pCurrComponent->IsRuntimeOnly())
					{
						continue;
					}

					pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
					{
						pCurrComponent->Save(pWriter);
					}
					pWriter->EndGroup();
				}
			}
			pWriter->EndGroup();
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CEntity::PostLoad(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesIdentifiersRemapper)
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

	E_RESULT_CODE CEntity::RemoveComponent(TypeId componentTypeId)
	{
		return mpEntityManager->RemoveComponent(mId, componentTypeId);
	}

	E_RESULT_CODE CEntity::RemoveComponents()
	{
		return mpEntityManager->RemoveComponents(mId);
	}

	bool CEntity::HasComponent(TypeId componentTypeId)
	{
		return mpEntityManager->HasComponent(mId, componentTypeId);
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


	/*!
		\brief CEntityRef's definition
	*/

	CEntityRef::CEntityRef(TPtr<IWorld> pWorld, TEntityId entityRef):
		ISerializable(), mpWorld(pWorld), mEntityRef(entityRef)
	{
	}

	E_RESULT_CODE CEntityRef::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_INVALID_ARGS;
		}

		if (!mpWorld)
		{
			return RC_FAIL;
		}

		mEntityRef = TEntityId::Invalid;
		mRefStr = pReader->GetString("ref_path");

		auto&& identifiers = Wrench::StringUtils::Split(mRefStr, ".");
		if (identifiers.empty())
		{
			return RC_OK;
		}

		CEntity* pCurrEntity = mpWorld->FindEntity(static_cast<TEntityId>(atoll(identifiers.front().c_str())));

		for (auto it = identifiers.cbegin() + 1; it != std::prev(identifiers.cend()); it++)
		{
			CTransform* pTransform = pCurrEntity->GetComponent<CTransform>();

			bool hasChildFound = false;

			for (TEntityId childEntityId : pTransform->GetChildren())
			{
				if (CEntity* pChildEntity = mpWorld->FindEntity(childEntityId))
				{
					if (pChildEntity->GetId() == static_cast<TEntityId>(atoll(identifiers.front().c_str())))
					{
						pCurrEntity = pChildEntity;
						mEntityRef = pChildEntity->GetId();

						break;
					}
				}
			}
		}

		return RC_OK;
	}
	
	E_RESULT_CODE CEntityRef::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		if (!mpWorld)
		{
			return RC_FAIL;
		}

		std::string serializedRefStr;

		CEntity* pCurrEntity = mpWorld->FindEntity(mEntityRef);
		
		while (pCurrEntity)
		{
			serializedRefStr = serializedRefStr + "." + std::to_string(static_cast<U32>(pCurrEntity->GetId()));

			CTransform* pTransform = pCurrEntity->GetComponent<CTransform>();
			pCurrEntity = mpWorld->FindEntity(pTransform->GetParent());
		}

		return pWriter->SetString("ref_path", serializedRefStr);
	}


	/*!
		\brief TEntitiesMapper's definition
	*/

	TEntityId TEntitiesMapper::Resolve(TEntityId input) const
	{
		auto&& it = mSerializedToRuntimeIdsTable.find(input);
		return it == mSerializedToRuntimeIdsTable.cend() ? input : it->second;
	}

	TEntityId TEntitiesMapper::Resolve(const CEntityRef& entityRef) const
	{
		TDE2_UNIMPLEMENTED();
		return TEntityId::Invalid;
	}
}