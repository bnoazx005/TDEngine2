#include "../../../include/graphics/UI/CLayoutElementComponent.h"


namespace TDEngine2
{
	struct TLayoutElementArchiveKeys
	{
	};


	CLayoutElement::CLayoutElement() :
		CBaseComponent(), mCanvasEntityId(TEntityId::Invalid), mIsDirty(true)
	{
	}

	E_RESULT_CODE CLayoutElement::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}
		
		return RC_OK;
	}

	E_RESULT_CODE CLayoutElement::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	void CLayoutElement::SetWorldRect(const TRectF32& rect)
	{
		mWorldRect = rect;
		mIsDirty = true;
	}

	void CLayoutElement::SetAnchorWorldRect(const TRectF32& rect)
	{
		mAnchorWorldRect = rect;
		mIsDirty = true;
	}

	void CLayoutElement::SetParentWorldRect(const TRectF32& rect)
	{
		mParentWorldRect = rect;
		mIsDirty = true;
	}

	void CLayoutElement::SetOwnerCanvasId(TEntityId canvasEntityId)
	{
		mCanvasEntityId = canvasEntityId;
	}

	void CLayoutElement::SetDirty(bool value)
	{
		mIsDirty = value;
	}

	E_RESULT_CODE CLayoutElement::SetMinAnchor(const TVector2& value)
	{
		if (value.x > 1.0 || value.x < 0.0)
		{
			return RC_INVALID_ARGS;
		}

		mMinAnchor = value;
		mIsDirty = true;

		return RC_OK;
	}

	E_RESULT_CODE CLayoutElement::SetMaxAnchor(const TVector2& value)
	{
		if (value.x > 1.0 || value.x < 0.0)
		{
			return RC_INVALID_ARGS;
		}

		mMaxAnchor = value;
		mIsDirty = true;

		return RC_OK;
	}

	E_RESULT_CODE CLayoutElement::SetMinOffset(const TVector2& value)
	{
		mMinOffset = value;
		mIsDirty = true;

		return RC_OK;
	}

	E_RESULT_CODE CLayoutElement::SetMaxOffset(const TVector2& value)
	{
		mMaxOffset = value;
		mIsDirty = true;

		return RC_OK;
	}

	E_RESULT_CODE CLayoutElement::SetPivot(const TVector2& value)
	{
		mPivot = value;
		mIsDirty = true;

		return RC_OK;
	}

	const TVector2& CLayoutElement::GetMinAnchor() const
	{
		return mMinAnchor;
	}

	const TVector2& CLayoutElement::GetMaxAnchor() const
	{
		return mMaxAnchor;
	}

	const TVector2& CLayoutElement::GetMinOffset() const
	{
		return mMinOffset;
	}

	const TVector2& CLayoutElement::GetMaxOffset() const
	{
		return mMaxOffset;
	}

	const TVector2& CLayoutElement::GetPivot() const
	{
		return mPivot;
	}

	const TRectF32& CLayoutElement::GetWorldRect() const
	{
		return mWorldRect;
	}

	const TRectF32& CLayoutElement::GetAnchorWorldRect() const
	{
		return mAnchorWorldRect;
	}

	const TRectF32& CLayoutElement::GetParentWorldRect() const
	{
		return mParentWorldRect;
	}

	TEntityId CLayoutElement::GetOwnerCanvasId() const
	{
		return mCanvasEntityId;
	}

	bool CLayoutElement::IsDirty() const
	{
		return mIsDirty;
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
}