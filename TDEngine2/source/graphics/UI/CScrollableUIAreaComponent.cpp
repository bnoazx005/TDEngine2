#include "../../../include/graphics/UI/CScrollableUIAreaComponent.h"
#include "../../../include/math/MathUtils.h"
#define META_EXPORT_ECS_SECTION
#include "../../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateScrollableUIAreaFactory)
	TDE2_DEFINE_COMPONENT_META(TScrollableUIAreaComponentData)


	CScrollableUIArea::CScrollableUIArea() :
		CBaseComponentT()
	{
	}

	E_RESULT_CODE CScrollableUIArea::PostLoad(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesIdentifiersRemapper)
	{
		mData.mContentEntityRef = entitiesIdentifiersRemapper.Resolve(mData.mContentEntityRef);
		
		return CBaseComponentT::PostLoad(pEntityManager, entitiesIdentifiersRemapper);
	}

	void CScrollableUIArea::SetContentEntityId(TEntityId cursorId)
	{
		mData.mContentEntityRef = cursorId;
	}

	void CScrollableUIArea::SetScrollSpeedFactor(F32 value)
	{
		mData.mScrollSpeedFactor = value;
	}

	void CScrollableUIArea::SetLayoutPrepared(bool value)
	{
		mData.mIsLayoutPrepared = value;
	}

	void CScrollableUIArea::SetHorizontal(bool state)
	{
		mData.mIsHorizontal = state;
	}

	void CScrollableUIArea::SetVertical(bool state)
	{
		mData.mIsVertical = state;
	}

	void CScrollableUIArea::SetEnabled(bool value)
	{
		mData.mIsEnabled = value;
	}

	void CScrollableUIArea::SetNormalizedScrollPosition(const TVector2& value)
	{
		mData.mNormalizedScrollPosition = TVector2(CMathUtils::Clamp01(value.x), CMathUtils::Clamp01(value.y));
	}
	
	TEntityId CScrollableUIArea::GetContentEntityId() const
	{
		return mData.mContentEntityRef;
	}

	F32 CScrollableUIArea::GetScrollSpeedFactor() const
	{
		return mData.mScrollSpeedFactor;
	}

	bool CScrollableUIArea::IsLayoutInitialized() const
	{
		return mData.mIsLayoutPrepared;
	}

	bool CScrollableUIArea::IsHorizontal() const
	{
		return mData.mIsHorizontal;
	}

	bool CScrollableUIArea::IsVertical() const
	{
		return mData.mIsVertical;
	}

	bool CScrollableUIArea::IsEnabled() const
	{
		return mData.mIsEnabled;
	}

	const TVector2& CScrollableUIArea::GetNormalizedScrollPosition() const
	{
		return mData.mNormalizedScrollPosition;
	}


	IComponent* CreateScrollableUIArea(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CScrollableUIArea, result);
	}


	/*!
		\brief CScrollableUIAreaFactory's definition
	*/

	CScrollableUIAreaFactory::CScrollableUIAreaFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CScrollableUIAreaFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateScrollableUIArea(result);
	}

	E_RESULT_CODE CScrollableUIAreaFactory::SetupComponent(CScrollableUIArea* pComponent, const TScrollableUIAreaParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateScrollableUIAreaFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CScrollableUIAreaFactory, result);
	}
}