#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CEntityManager.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/scene/components/CObjIdComponent.h"
#include "../../include/editor/ecs/EditorComponents.h"
#include <memory>
#include <queue>
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

		mPathIdentifiers.clear();

		for (auto&& currId : Wrench::StringUtils::Split(mRefStr, "."))
		{
			mPathIdentifiers.push_back(static_cast<U32>(atoll(currId.c_str())));
		}

		return RC_OK;
	}


	static inline void AppendPath(CEntity* pCurrEntity, std::string& pathOutput)
	{
		static const std::string delimiter = ".";

		const bool isPathEmpty = pathOutput.empty();

		if (auto pObjIdComponent = pCurrEntity->GetComponent<CObjIdComponent>())
		{
			pathOutput = std::to_string(pObjIdComponent->mId) + (isPathEmpty ? Wrench::StringUtils::GetEmptyStr() : delimiter) + pathOutput;
			return;
		}

		pathOutput = std::to_string(static_cast<U32>(pCurrEntity->GetId())) + (isPathEmpty ? Wrench::StringUtils::GetEmptyStr() : delimiter) + pathOutput;
	}


	static CEntity* TraverseUpToNextLink(TPtr<IWorld> pWorld, CEntity* pCurrEntity)
	{
		CEntity* pLinkEntity = pCurrEntity;

#if TDE2_EDITORS_ENABLED
		while (pLinkEntity && !pLinkEntity->HasComponent<CPrefabLinkInfoComponent>())
		{
			auto pTransform = pLinkEntity->GetComponent<CTransform>();
			pLinkEntity = pWorld->FindEntity(pTransform->GetParent());
		}
#endif

		return pLinkEntity;
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

		if (TEntityId::Invalid == mEntityRef)
		{
			return RC_OK;
		}

		CEntity* pCurrEntity = mpWorld->FindEntity(mEntityRef);
		std::string serializedRefStr;
		
		/// \note Traverse the hierarchy up to a root and add identifiers of links to the path
		while (pCurrEntity)
		{
			AppendPath(pCurrEntity, serializedRefStr);
			pCurrEntity = TraverseUpToNextLink(mpWorld, pCurrEntity);
		}

		return pWriter->SetString("ref_path", serializedRefStr);
	}


	static CEntity* FindEntityWithObjId(TPtr<IWorld> pWorld, CEntity* pRootEntity, U32 objectId)
	{
		std::queue<CEntity*> entitiesToProcess;

		if (CTransform* pTransform = pRootEntity->GetComponent<CTransform>())
		{
			for (auto&& currChildId : pTransform->GetChildren())
			{
				entitiesToProcess.emplace(pWorld->FindEntity(currChildId));
			}
		}

		while (!entitiesToProcess.empty())
		{
			CEntity* pCurrEntity = entitiesToProcess.front();
			entitiesToProcess.pop();

			if (auto pObjIdComponent = pCurrEntity->GetComponent<CObjIdComponent>())
			{
				if (pObjIdComponent->mId == objectId)
				{
					return pCurrEntity;
				}
			}

			if (auto pTransform = pCurrEntity->GetComponent<CTransform>())
			{
				for (auto&& currChildId : pTransform->GetChildren())
				{
					entitiesToProcess.emplace(pWorld->FindEntity(currChildId));
				}
			}
		}

		return nullptr;
	}


	void CEntityRef::Set(TEntityId ref)
	{
		mEntityRef = ref;
	}

	TDE2_API TEntityId CEntityRef::Get()
	{
		if (TEntityId::Invalid != mEntityRef)
		{
			return mEntityRef;
		}

		/// \note Firstly we try to find the rightmost identifier that could be found using IWorld::FindEntity
		auto it = mPathIdentifiers.begin();

		CEntity* pLastAccessibleEntity = nullptr;
		CEntity* pCurrEntity = nullptr;

		while (it != mPathIdentifiers.end() && (pCurrEntity = mpWorld->FindEntity(static_cast<TEntityId>(*it))))
		{
			pLastAccessibleEntity = pCurrEntity;
			it++;
		}

		pCurrEntity = pLastAccessibleEntity;

		/// \note Beginning from that we iterate over children in topdown manner and check either via IWorld::FindEntity or CObjIdComponent's component
		for (; it != mPathIdentifiers.cend(); it++)
		{
			pCurrEntity = FindEntityWithObjId(mpWorld, pCurrEntity, *it);
			if (!pCurrEntity)
			{
				return TEntityId::Invalid; /// \note If we haven't found entity it means resolving failed 
			}
		}

		TDE2_ASSERT(pCurrEntity);
		mEntityRef = pCurrEntity ? pCurrEntity->GetId() : TEntityId::Invalid;

		return mEntityRef;
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