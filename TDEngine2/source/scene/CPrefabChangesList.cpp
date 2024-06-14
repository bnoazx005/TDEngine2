#include "../../include/scene/CPrefabChangesList.h"
#include "../../include/scene/IPrefabsRegistry.h"
#include "../../include/scene/components/CPrefabLinkInfoComponent.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CEntityManager.h"
#include "../../include/ecs/IComponent.h"
#include "../../include/utils/CFileLogger.h"
#include "stringUtils.hpp"
#include <algorithm>


namespace TDEngine2
{
	struct TPrefabChangesListArchiveKeys
	{
		static const std::string mChangesArrayGroupKey;
		static const std::string mChangesArrayElementGroupKey;
		static const std::string mTargetIdLey;
		static const std::string mPropertyBindingKey;
		static const std::string mValueKey;
	};

	const std::string TPrefabChangesListArchiveKeys::mChangesArrayGroupKey = "changes";
	const std::string TPrefabChangesListArchiveKeys::mChangesArrayElementGroupKey = "element";
	const std::string TPrefabChangesListArchiveKeys::mTargetIdLey = "target_id";
	const std::string TPrefabChangesListArchiveKeys::mPropertyBindingKey = "property_binding";
	const std::string TPrefabChangesListArchiveKeys::mValueKey = "value";


	CPrefabChangesList::CPrefabChangesList() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CPrefabChangesList::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}
		
		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CPrefabChangesList::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		if (mChanges.empty())
		{
			return RC_OK; /// \note Do nothing 'cause there are no changes
		}

		pWriter->BeginGroup(TPrefabChangesListArchiveKeys::mChangesArrayGroupKey);
		
		for (TChangeDesc& currDesc : mChanges)
		{
			pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
			{
				pWriter->BeginGroup(TPrefabChangesListArchiveKeys::mChangesArrayElementGroupKey);

				SaveEntityRef(pWriter, TPrefabChangesListArchiveKeys::mTargetIdLey, currDesc.mTargetLinkEntityId);
				pWriter->SetString(TPrefabChangesListArchiveKeys::mPropertyBindingKey, currDesc.mPropertyBinding);

				pWriter->BeginGroup(TPrefabChangesListArchiveKeys::mValueKey);
				currDesc.mValue.Save(pWriter);
				pWriter->EndGroup();


				pWriter->EndGroup();
			}
			pWriter->EndGroup();
		}
		
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CPrefabChangesList::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		mChanges.clear();

		pReader->BeginGroup(TPrefabChangesListArchiveKeys::mChangesArrayGroupKey);

		TChangeDesc currDesc;

		while (pReader->HasNextItem())
		{
			pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
			{
				pReader->BeginGroup(TPrefabChangesListArchiveKeys::mChangesArrayElementGroupKey);

				currDesc.mTargetLinkEntityId = LoadEntityRef(pReader,TPrefabChangesListArchiveKeys::mTargetIdLey);
				currDesc.mPropertyBinding = pReader->GetString(TPrefabChangesListArchiveKeys::mPropertyBindingKey);
				
				pReader->BeginGroup(TPrefabChangesListArchiveKeys::mValueKey);
				auto loadValueResult = DeserializeValue(pReader);
				pReader->EndGroup();

				if (loadValueResult.HasError())
				{
					result = result | loadValueResult.GetError();
				}
				else
				{
					currDesc.mValue = loadValueResult.Get();
				}

				pReader->EndGroup();

				mChanges.emplace_back(currDesc);
			}
			pReader->EndGroup();
		}

		pReader->EndGroup();

		return result;
	}

	TPtr<CPrefabChangesList> CPrefabChangesList::Clone() const
	{
		E_RESULT_CODE result = RC_OK;

		auto pChangesList = TPtr<CPrefabChangesList>(CreatePrefabChangesList(result));

		std::copy(mChanges.begin(), mChanges.end(), std::back_inserter(pChangesList->mChanges));

		return pChangesList;
	}

#if TDE2_EDITORS_ENABLED

	E_RESULT_CODE CPrefabChangesList::AddModification(const TChangeDesc& desc)
	{
		auto it = std::find_if(mChanges.begin(), mChanges.end(), [&desc](auto&& currElement) 
		{ 
			return currElement.mTargetLinkEntityId == desc.mTargetLinkEntityId && currElement.mPropertyBinding == desc.mPropertyBinding;
		});

		if (it == mChanges.end())
		{
			mChanges.emplace_back(desc);
			return RC_OK;
		}

		*it = desc;

		return RC_OK;
	}

	E_RESULT_CODE CPrefabChangesList::RemoveModification(const CEntityRef& target, const std::string& propertyBinding)
	{
		auto it = std::find_if(mChanges.begin(), mChanges.end(), [&target, &propertyBinding](auto&& currElement)
		{
			return target == currElement.mTargetLinkEntityId && propertyBinding == currElement.mPropertyBinding;
		});

		if (it == mChanges.end())
		{
			return RC_FAIL;
		}

		mChanges.erase(it);
		return RC_OK;
	}

#endif

	E_RESULT_CODE CPrefabChangesList::ApplyChanges(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesMappings)
	{
		if (!pEntityManager)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		for (auto&& currChangeEntry : mChanges)
		{
			auto& targetLinkRef = currChangeEntry.mTargetLinkEntityId;

			targetLinkRef.PostLoad(pEntityManager, entitiesMappings);
			targetLinkRef.Set(TEntityId::Invalid);
			
			auto pProperty = ResolveBinding(pEntityManager, pEntityManager->GetEntity(targetLinkRef.Get()).Get(), currChangeEntry.mPropertyBinding);
			if (!pProperty)
			{
				LOG_WARNING(Wrench::StringUtils::Format("[CPrefabChangesList][ApplyChanges] Can't resolve binding {0}", currChangeEntry.mPropertyBinding));
			}

			result = result | currChangeEntry.mValue.Apply(pProperty);
		}

		return result;
	}


	TDE2_API CPrefabChangesList* CreatePrefabChangesList(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CPrefabChangesList, CPrefabChangesList, result);
	}


#if TDE2_EDITORS_ENABLED

	struct TEntityInfo
	{
		TEntityId mEntityId = TEntityId::Invalid;
		std::unordered_map<TypeId, std::tuple<std::string, IPropertyWrapperPtr>> mPropertiesTable {};
	};

	using TPropertiesTable = std::unordered_map<std::string, TEntityInfo>;


	static TPropertiesTable GetPropertiesTable(CEntityManager* pEntityManager, TEntityId rootEntityId)
	{
		TPropertiesTable instancePropertiesTable;

		/// \note Iterate over all entities/components of rootEntityId's hierarchy 
		TraverseEntityHierarchy(pEntityManager, rootEntityId, [pEntityManager, &instancePropertiesTable](auto&& pCurrEntity)
		{
			auto& currEntityTable = instancePropertiesTable[GetEntityPath(pEntityManager, pCurrEntity->GetId())];
			currEntityTable.mEntityId = pCurrEntity->GetId();

			auto& propertiesTable = currEntityTable.mPropertiesTable;

			for (auto pCurrComponent : pCurrEntity->GetComponents())
			{
				if (pCurrComponent->GetComponentTypeId() == CPrefabLinkInfoComponent::GetTypeId() || pCurrComponent->IsRuntimeOnly())
				{
					continue;
				}

				for (const std::string& currPropertyName : pCurrComponent->GetAllProperties())
				{
					propertiesTable.emplace(
						pCurrComponent->GetComponentTypeId(), 
						std::make_tuple(pCurrComponent->GetTypeName() + "." + currPropertyName, pCurrComponent->GetProperty(currPropertyName)));
				}
			}

			if (pCurrEntity->template HasComponent<CPrefabLinkInfoComponent>())
			{
				return false; /// \note Don't traverse hierarchy underneath cause it's another prefab
			}

			return true;
		});

		return std::move(instancePropertiesTable);
	}


	static TPropertiesTable FindModificationsInInstance(const TPtr<IPrefabsRegistry>& pPrefabsRegistry, CEntityManager* pEntityManager, TEntityId rootEntityId)
	{
		auto pInstanceRootEntity = pEntityManager->GetEntity(rootEntityId);
		if (!pInstanceRootEntity)
		{
			return {};
		}

		auto pPrefabLinkInfo = pInstanceRootEntity->GetComponent<CPrefabLinkInfoComponent>();
		if (!pPrefabLinkInfo)
		{
			return {};
		}

		/// \note Retrieve original prefab's hierarchy
		auto pPrefabInfo = pPrefabsRegistry->GetPrefabInfo(pPrefabLinkInfo->GetPrefabLinkId());
		if (!pPrefabInfo)
		{
			return {};
		}

		/// \note Retrieve properties of all components of original prefab and current instance's one
		auto&& prefabHierarchyPropsTable = GetPropertiesTable(pPrefabInfo->mpEntityOwner, pPrefabInfo->mRootEntityId);
		auto&& currInstanceHierarchyPropsTable = GetPropertiesTable(pEntityManager, rootEntityId);

		for (auto&& currEntry : prefabHierarchyPropsTable)
		{
			auto it = currInstanceHierarchyPropsTable.find(currEntry.first);
			if (it == currInstanceHierarchyPropsTable.cend())
			{
				continue;
			}

			auto&& instanceComponentPropertyTable = it->second.mPropertiesTable;

			for (auto&& originalComponentPropertyPair : currEntry.second.mPropertiesTable)
			{
				auto propertyIt = instanceComponentPropertyTable.find(originalComponentPropertyPair.first);
				if (propertyIt == instanceComponentPropertyTable.cend())
				{
					continue;
				}

				if (std::get<IPropertyWrapperPtr>(propertyIt->second)->EqualsTo(std::get<IPropertyWrapperPtr>(originalComponentPropertyPair.second)))
				{
					instanceComponentPropertyTable.erase(propertyIt); /// \note Remove property if it's same in two tables
				}
			}
		}

		return currInstanceHierarchyPropsTable;
	}


	TDE2_API E_RESULT_CODE StoreChanges(TPtr<CPrefabChangesList>& pPrefabChanges, const TPtr<IPrefabsRegistry>& pPrefabsRegistry, CEntityManager* pEntityManager, TEntityId rootEntityId)
	{
		if (!pPrefabChanges || !pEntityManager || !pPrefabsRegistry || TEntityId::Invalid == rootEntityId)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		/// \note iterate through modifications and add them into pPrefabChanges
		for (auto&& currPropertyModification : FindModificationsInInstance(pPrefabsRegistry, pEntityManager, rootEntityId))
		{
			const TEntityId& currEntityId = currPropertyModification.second.mEntityId;

			for (auto&& currProperty : currPropertyModification.second.mPropertiesTable)
			{
				CValueWrapper value;
				result = result | value.Set(std::get<IPropertyWrapperPtr>(currProperty.second));

				result = result | pPrefabChanges->AddModification({CEntityRef(pEntityManager, currEntityId), std::get<std::string>(currProperty.second), value });
			}
		}

		return result;
	}

#endif
}