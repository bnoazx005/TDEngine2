#include "../../../include/graphics/UI/CLayoutElementComponent.h"


namespace TDEngine2
{
	struct TLayoutElementArchiveKeys
	{
		static const std::string mMinAnchorKeyId;
		static const std::string mMaxAnchorKeyId;
		
		static const std::string mMinOffsetKeyId;
		static const std::string mMaxOffsetKeyId;
		
		static const std::string mPivotKeyId;
	};


	const std::string TLayoutElementArchiveKeys::mMinAnchorKeyId = "min_anchor";
	const std::string TLayoutElementArchiveKeys::mMaxAnchorKeyId = "max_anchor";

	const std::string TLayoutElementArchiveKeys::mMinOffsetKeyId = "min_offset";
	const std::string TLayoutElementArchiveKeys::mMaxOffsetKeyId = "max_offset";

	const std::string TLayoutElementArchiveKeys::mPivotKeyId = "pivot";


	CLayoutElement::CLayoutElement() :
		CBaseComponent(), 
		mCanvasEntityId(TEntityId::Invalid), 
		mIsDirty(true),
		mMinAnchor(ZeroVector2),
		mMaxAnchor(ZeroVector2),
		mMinOffset(ZeroVector2),
		mMaxOffset(TVector2(100.0f))
	{
	}

	E_RESULT_CODE CLayoutElement::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		/// \note Anchors
		pReader->BeginGroup(TLayoutElementArchiveKeys::mMinAnchorKeyId);
		if (auto value = LoadVector2(pReader))
		{
			mMinAnchor = value.Get();
		}
		pReader->EndGroup();

		pReader->BeginGroup(TLayoutElementArchiveKeys::mMaxAnchorKeyId);
		if (auto value = LoadVector2(pReader))
		{
			mMaxAnchor = value.Get();
		}
		pReader->EndGroup();

		/// \note Offsets
		pReader->BeginGroup(TLayoutElementArchiveKeys::mMinOffsetKeyId);
		if (auto value = LoadVector2(pReader))
		{
			mMinOffset = value.Get();
		}
		pReader->EndGroup();

		pReader->BeginGroup(TLayoutElementArchiveKeys::mMaxOffsetKeyId);
		if (auto value = LoadVector2(pReader))
		{
			mMaxOffset = value.Get();
		}
		pReader->EndGroup();

		/// \note Pivot
		pReader->BeginGroup(TLayoutElementArchiveKeys::mPivotKeyId);
		if (auto value = LoadVector2(pReader))
		{
			mPivot = value.Get();
		}
		pReader->EndGroup();

		mIsDirty = true;
		
		return RC_OK;
	}

	E_RESULT_CODE CLayoutElement::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CLayoutElement::GetTypeId()));

			/// \note Anchors
			pWriter->BeginGroup(TLayoutElementArchiveKeys::mMinAnchorKeyId, false);
			SaveVector2(pWriter, mMinAnchor);
			pWriter->EndGroup();

			pWriter->BeginGroup(TLayoutElementArchiveKeys::mMaxAnchorKeyId, false);
			SaveVector2(pWriter, mMaxAnchor);
			pWriter->EndGroup();

			/// \note Offsets
			pWriter->BeginGroup(TLayoutElementArchiveKeys::mMinOffsetKeyId, false);
			SaveVector2(pWriter, mMinOffset);
			pWriter->EndGroup();

			pWriter->BeginGroup(TLayoutElementArchiveKeys::mMaxOffsetKeyId, false);
			SaveVector2(pWriter, mMaxOffset);
			pWriter->EndGroup();

			/// \note Pivot
			pWriter->BeginGroup(TLayoutElementArchiveKeys::mPivotKeyId, false);
			SaveVector2(pWriter, mPivot);
			pWriter->EndGroup();
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CLayoutElement::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<CLayoutElement*>(pDestObject))
		{
			pComponent->mAnchorWorldRect = mAnchorWorldRect;
			pComponent->mCanvasEntityId = mCanvasEntityId;
			pComponent->mMaxAnchor = mMaxAnchor;
			pComponent->mMaxOffset = mMaxOffset;
			pComponent->mMinAnchor = mMinAnchor;
			pComponent->mMinOffset = mMinOffset;
			pComponent->mParentWorldRect = mParentWorldRect;
			pComponent->mPivot = mPivot;
			pComponent->mWorldRect = mWorldRect;

			pComponent->mIsDirty = true;

			return RC_OK;
		}

		return RC_FAIL;
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
		if (value.x > 1.0f || value.x < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		mMinAnchor = value;
		mIsDirty = true;

		return RC_OK;
	}

	E_RESULT_CODE CLayoutElement::SetMaxAnchor(const TVector2& value)
	{
		if (value.x > 1.0f || value.x < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		mMaxAnchor = TVector2(CMathUtils::Max(mMinAnchor.x, value.x), CMathUtils::Max(mMinAnchor.y, value.y));
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