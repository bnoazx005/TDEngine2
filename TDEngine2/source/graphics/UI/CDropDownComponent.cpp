#include "../../../include/graphics/UI/CDropDownComponent.h"
#include <algorithm>
#define META_EXPORT_ECS_SECTION
#include "../../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateDropDownFactory)
	TDE2_DEFINE_COMPONENT_META(TDropDownComponentData)


	CDropDown::CDropDown() :
		CBaseComponentT()
	{
	}


	E_RESULT_CODE CDropDown::PostLoad(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesIdentifiersRemapper)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);

		mData.mLabelEntityRef = entitiesIdentifiersRemapper.Resolve(mData.mLabelEntityRef);
		mData.mPopupRootEntityRef = entitiesIdentifiersRemapper.Resolve(mData.mPopupRootEntityRef);
		mData.mContentEntityRef = entitiesIdentifiersRemapper.Resolve(mData.mContentEntityRef);
		mData.mItemPrefabEntityRef = entitiesIdentifiersRemapper.Resolve(mData.mItemPrefabEntityRef);

		return CBaseComponentT::PostLoad(pEntityManager, entitiesIdentifiersRemapper);
	}

	void CDropDown::SetLabelEntityId(TEntityId labelId)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		mData.mLabelEntityRef = labelId;
	}

	void CDropDown::SetPopupRootEntityId(TEntityId entityId)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		mData.mPopupRootEntityRef = entityId;
	}

	void CDropDown::SetContentEntityId(TEntityId contentEntityId)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		mData.mContentEntityRef = contentEntityId;
	}

	void CDropDown::SetItemPrefabEntityId(TEntityId entityId)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		mData.mItemPrefabEntityRef = entityId;
	}

	E_RESULT_CODE CDropDown::SetSelectedItem(U32 index)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);

		if (index >= mData.mItems.size())
		{
			return RC_INVALID_ARGS;
		}

		mData.mSelectedItemIndex = index;

		return RC_OK;
	}

	void CDropDown::SetItems(const TDropDownComponentData::TOptionsArray& items)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		mData.mItems = std::move(items);
	}

	void CDropDown::SetExpanded(bool state)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		mData.mIsExpanded = state;
	}

	TEntityId CDropDown::GetLabelEntityId() const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		return mData.mLabelEntityRef;
	}

	TEntityId CDropDown::GetPopupRootEntityId() const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		return mData.mPopupRootEntityRef;
	}

	TEntityId CDropDown::GetContentEntityId() const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		return mData.mContentEntityRef;
	}

	TEntityId CDropDown::GetItemPrefabEntityId() const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		return mData.mItemPrefabEntityRef;
	}

	const TDropDownComponentData::TOptionsArray& CDropDown::GetItems() const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		return mData.mItems;
	}

	U32 CDropDown::GetSelectedItem() const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		return mData.mSelectedItemIndex;
	}

	TEntitiesArray& CDropDown::GetItemsEntities()
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		return mData.mItemsEntities;
	}

	bool CDropDown::IsExpanded() const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		return mData.mIsExpanded;
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