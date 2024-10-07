#include "../../../include/graphics/UI/CDropDownComponent.h"
#include <algorithm>


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateDropDownFactory)


	CDropDown::CDropDown() :
		CBaseComponent()
	{
	}


	struct TDropDownArchiveKeys
	{
		static const std::string mLabelEntityRefKeyId;
		static const std::string mPopupRootEntityRefKeyId;
		static const std::string mContentEntityRefKeyId;
		static const std::string mItemPrefabEntityRefKeyId;
		static const std::string mItemsKeyId;
		static const std::string mSingleItemKeyId;
		static const std::string mSelectedItemKeyId;
	};


	const std::string TDropDownArchiveKeys::mLabelEntityRefKeyId = "label_entity_ref";
	const std::string TDropDownArchiveKeys::mPopupRootEntityRefKeyId = "popup_root_entity_ref";
	const std::string TDropDownArchiveKeys::mContentEntityRefKeyId = "content_entity_ref";
	const std::string TDropDownArchiveKeys::mItemPrefabEntityRefKeyId = "item_prefab_ref";
	const std::string TDropDownArchiveKeys::mItemsKeyId = "items";
	const std::string TDropDownArchiveKeys::mSingleItemKeyId = "item";
	const std::string TDropDownArchiveKeys::mSelectedItemKeyId = "selected_item";


	E_RESULT_CODE CDropDown::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);

		mLabelEntityRef = static_cast<TEntityId>(pReader->GetUInt32(TDropDownArchiveKeys::mLabelEntityRefKeyId));
		mPopupRootEntityRef = static_cast<TEntityId>(pReader->GetUInt32(TDropDownArchiveKeys::mPopupRootEntityRefKeyId));
		mContentEntityRef = static_cast<TEntityId>(pReader->GetUInt32(TDropDownArchiveKeys::mContentEntityRefKeyId));
		mItemPrefabEntityRef = static_cast<TEntityId>(pReader->GetUInt32(TDropDownArchiveKeys::mItemPrefabEntityRefKeyId));

		mSelectedItemIndex = pReader->GetUInt32(TDropDownArchiveKeys::mSelectedItemKeyId, 0);

		mItems.clear();

		pReader->BeginGroup(TDropDownArchiveKeys::mItemsKeyId);
		
		while (pReader->HasNextItem())
		{
			pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
			mItems.emplace_back(pReader->GetString(TDropDownArchiveKeys::mSingleItemKeyId));
			pReader->EndGroup();
		}
		
		pReader->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CDropDown::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CDropDown::GetTypeId()));

			pWriter->SetUInt32(TDropDownArchiveKeys::mLabelEntityRefKeyId, static_cast<U32>(mLabelEntityRef));
			pWriter->SetUInt32(TDropDownArchiveKeys::mPopupRootEntityRefKeyId, static_cast<U32>(mPopupRootEntityRef));
			pWriter->SetUInt32(TDropDownArchiveKeys::mContentEntityRefKeyId, static_cast<U32>(mContentEntityRef));
			pWriter->SetUInt32(TDropDownArchiveKeys::mItemPrefabEntityRefKeyId, static_cast<U32>(mItemPrefabEntityRef));

			pWriter->SetUInt32(TDropDownArchiveKeys::mSelectedItemKeyId, mSelectedItemIndex);

			pWriter->BeginGroup(TDropDownArchiveKeys::mItemsKeyId);
			
			for (auto&& currItem : mItems)
			{
				pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
				pWriter->SetString(TDropDownArchiveKeys::mSingleItemKeyId, currItem);
				pWriter->EndGroup();
			}

			pWriter->EndGroup();
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CDropDown::PostLoad(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesIdentifiersRemapper)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);

		mLabelEntityRef = entitiesIdentifiersRemapper.Resolve(mLabelEntityRef);
		mPopupRootEntityRef = entitiesIdentifiersRemapper.Resolve(mPopupRootEntityRef);
		mContentEntityRef = entitiesIdentifiersRemapper.Resolve(mContentEntityRef);
		mItemPrefabEntityRef = entitiesIdentifiersRemapper.Resolve(mItemPrefabEntityRef);

		return CBaseComponent::PostLoad(pEntityManager, entitiesIdentifiersRemapper);
	}

	E_RESULT_CODE CDropDown::Clone(IComponent*& pDestObject) const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);

		if (auto pComponent = dynamic_cast<CDropDown*>(pDestObject))
		{
			pComponent->mLabelEntityRef = mLabelEntityRef;
			pComponent->mPopupRootEntityRef = mPopupRootEntityRef;
			pComponent->mContentEntityRef = mContentEntityRef;
			pComponent->mItemPrefabEntityRef = mItemPrefabEntityRef;
			pComponent->mSelectedItemIndex = mSelectedItemIndex;

			pComponent->mItems.clear();
			std::copy(mItems.cbegin(), mItems.cend(), std::back_inserter(pComponent->mItems));

			return RC_OK;
		}

		return RC_FAIL;
	}

	void CDropDown::SetLabelEntityId(TEntityId labelId)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);
		mLabelEntityRef = labelId;
	}

	void CDropDown::SetPopupRootEntityId(TEntityId entityId)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);
		mPopupRootEntityRef = entityId;
	}

	void CDropDown::SetContentEntityId(TEntityId contentEntityId)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);
		mContentEntityRef = contentEntityId;
	}

	void CDropDown::SetItemPrefabEntityId(TEntityId entityId)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);
		mItemPrefabEntityRef = entityId;
	}

	E_RESULT_CODE CDropDown::SetSelectedItem(U32 index)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);

		if (index >= mItems.size())
		{
			return RC_INVALID_ARGS;
		}

		mSelectedItemIndex = index;

		return RC_OK;
	}

	void CDropDown::SetItems(const TOptionsArray& items)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);
		mItems = std::move(items);
	}

	void CDropDown::SetExpanded(bool state)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);
		mIsExpanded = state;
	}

	TEntityId CDropDown::GetLabelEntityId() const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);
		return mLabelEntityRef;
	}

	TEntityId CDropDown::GetPopupRootEntityId() const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);
		return mPopupRootEntityRef;
	}

	TEntityId CDropDown::GetContentEntityId() const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);
		return mContentEntityRef;
	}

	TEntityId CDropDown::GetItemPrefabEntityId() const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);
		return mItemPrefabEntityRef;
	}

	const CDropDown::TOptionsArray& CDropDown::GetItems() const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);
		return mItems;
	}

	U32 CDropDown::GetSelectedItem() const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);
		return mSelectedItemIndex;
	}

	std::vector<TEntityId>& CDropDown::GetItemsEntities()
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);
		return mItemsEntities;
	}

	bool CDropDown::IsExpanded() const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(debugLock, mMTCheckLock);
		return mIsExpanded;
	}


	IComponent* CreateDropDown(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CDropDown, result);
	}


	/*!
		\brief CDropDownFactory's definition
	*/

	CDropDownFactory::CDropDownFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CDropDownFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateDropDown(result);
	}

	E_RESULT_CODE CDropDownFactory::SetupComponent(CDropDown* pComponent, const TDropDownParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateDropDownFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CDropDownFactory, result);
	}
}