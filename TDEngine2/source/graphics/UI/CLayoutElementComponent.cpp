#include "../../../include/graphics/UI/CLayoutElementComponent.h"
#define META_EXPORT_ECS_SECTION
#include "../../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateLayoutElementFactory)
	TDE2_DEFINE_COMPONENT_META(TLayoutElementComponentData)


	CLayoutElement::CLayoutElement() :
		CBaseComponentT()
	{
	}

	void CLayoutElement::SetWorldRect(const TRectF32& rect)
	{
		mData.mWorldRect = rect;
		mData.mIsDirty = true;
	}

	void CLayoutElement::SetAnchorWorldRect(const TRectF32& rect)
	{
		mData.mAnchorWorldRect = rect;
		mData.mIsDirty = true;
	}

	void CLayoutElement::SetParentWorldRect(const TRectF32& rect)
	{
		mData.mParentWorldRect = rect;
		mData.mIsDirty = true;
	}

	void CLayoutElement::SetOwnerCanvasId(TEntityId canvasEntityId)
	{
		mData.mCanvasEntityId = canvasEntityId;
	}

	void CLayoutElement::SetDirty(bool value)
	{
		mData.mIsDirty = value;
	}

	void CLayoutElement::SetIsPositionOffsetUsed(bool value)
	{
		mData.mIsPositionOffsetApplied = value;
		
		if (!value)
		{
			mData.mPositionOffset = ZeroVector2;
		}
	}

	E_RESULT_CODE CLayoutElement::SetScale(const TVector2& scale)
	{
		mData.mScale = scale;
		return RC_OK;
	}

	E_RESULT_CODE CLayoutElement::SetRotationAngle(F32 angle)
	{
		mData.mRotationAngle = angle;
		return RC_OK;
	}

	E_RESULT_CODE CLayoutElement::SetMinAnchor(const TVector2& value)
	{
		if (value.x > 1.0f || value.x < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		mData.mMinAnchor = value;
		mData.mIsDirty = true;

		return RC_OK;
	}

	E_RESULT_CODE CLayoutElement::SetMaxAnchor(const TVector2& value)
	{
		if (value.x > 1.0f || value.x < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		mData.mMaxAnchor = TVector2(CMathUtils::Max(mData.mMinAnchor.x, value.x), CMathUtils::Max(mData.mMinAnchor.y, value.y));
		mData.mIsDirty = true;

		return RC_OK;
	}

	E_RESULT_CODE CLayoutElement::SetMinOffset(const TVector2& value)
	{
		mData.mMinOffset = value;
		mData.mIsDirty = true;

		return RC_OK;
	}

	E_RESULT_CODE CLayoutElement::SetMaxOffset(const TVector2& value)
	{
		mData.mMaxOffset = value;
		mData.mIsDirty = true;

		return RC_OK;
	}

	E_RESULT_CODE CLayoutElement::SetPivot(const TVector2& value)
	{
		mData.mPivot = value;
		mData.mIsDirty = true;

		return RC_OK;
	}

	void CLayoutElement::SetPositionOffset(const TVector2& value)
	{
		mData.mPositionOffset = value;

		mData.mIsPositionOffsetApplied = true;
		mData.mIsDirty = true;
	}

	const TVector2& CLayoutElement::GetMinAnchor() const
	{
		return mData.mMinAnchor;
	}

	const TVector2& CLayoutElement::GetMaxAnchor() const
	{
		return mData.mMaxAnchor;
	}

	const TVector2& CLayoutElement::GetMinOffset() const
	{
		return mData.mMinOffset;
	}

	const TVector2& CLayoutElement::GetMaxOffset() const
	{
		return mData.mMaxOffset;
	}

	const TVector2& CLayoutElement::GetPivot() const
	{
		return mData.mPivot;
	}

	const TVector2& CLayoutElement::GetPositionOffset() const
	{
		return mData.mPositionOffset;
	}
	
	bool CLayoutElement::IsPositionOffsetUsed() const
	{
		return mData.mIsPositionOffsetApplied;
	}

	const TRectF32& CLayoutElement::GetWorldRect() const
	{
		return mData.mWorldRect;
	}

	const TRectF32& CLayoutElement::GetAnchorWorldRect() const
	{
		return mData.mAnchorWorldRect;
	}

	const TRectF32& CLayoutElement::GetParentWorldRect() const
	{
		return mData.mParentWorldRect;
	}

	TEntityId CLayoutElement::GetOwnerCanvasId() const
	{
		return mData.mCanvasEntityId;
	}

	bool CLayoutElement::IsDirty() const
	{
		return mData.mIsDirty;
	}

	const TVector2& CLayoutElement::GetScale() const
	{
		return mData.mScale;
	}
	
	F32 CLayoutElement::GetRotationAngle() const
	{
		return mData.mRotationAngle;
	}

	const std::string& CLayoutElement::GetTypeName() const
	{
		static const std::string typeName = "layout_element";
		return typeName;
	}


	IComponent* CreateLayoutElement(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CLayoutElement, result);
	}


	/*!
		\brief CLayoutElementFactory's definition
	*/

	CLayoutElementFactory::CLayoutElementFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CLayoutElementFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateLayoutElement(result);
	}

	E_RESULT_CODE CLayoutElementFactory::SetupComponent(CLayoutElement* pComponent, const TLayoutElementParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateLayoutElementFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CLayoutElementFactory, result);
	}


	TDE2_API TVector2 CalcContentRectAlignByType(const TRectF32& holderRect, const TRectF32& contentRect, E_UI_ELEMENT_ALIGNMENT_TYPE alignType)
	{
		if (E_UI_ELEMENT_ALIGNMENT_TYPE::LEFT_BOTTOM == alignType)
		{
			return ZeroVector2;
		}

		const TVector2 delta = holderRect.GetLeftBottom() - contentRect.GetLeftBottom();
		const TVector2 halfDelta = TVector2(0.5f * CMathUtils::Abs(delta.x), 0.5f * CMathUtils::Abs(delta.y));

		switch (alignType)
		{
			case E_UI_ELEMENT_ALIGNMENT_TYPE::BOTTOM:
				return TVector2(halfDelta.x, 0.0f);
			case E_UI_ELEMENT_ALIGNMENT_TYPE::LEFT:
				return TVector2(0.0f, halfDelta.y);
			case E_UI_ELEMENT_ALIGNMENT_TYPE::LEFT_TOP:
				return TVector2(0.0f, delta.y);
			case E_UI_ELEMENT_ALIGNMENT_TYPE::RIGHT:
				return TVector2(delta.x, halfDelta.y);
			case E_UI_ELEMENT_ALIGNMENT_TYPE::RIGHT_BOTTOM:
				return TVector2(delta.x, 0.0f);
			case E_UI_ELEMENT_ALIGNMENT_TYPE::RIGHT_TOP:
				return delta;
			case E_UI_ELEMENT_ALIGNMENT_TYPE::TOP:
				return TVector2(halfDelta.x, delta.y);
		}

		TDE2_UNREACHABLE();
		return ZeroVector2;
	}


	TDE2_DEFINE_FLAG_COMPONENT(UIMaskComponent);
}