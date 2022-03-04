#include "../../../include/graphics/UI/GroupLayoutComponents.h"
#include "../../../include/math/MathUtils.h"


namespace TDEngine2
{
	/*struct TGridGroupLayoutArchiveKeys
	{
		static const std::string mMinAnchorKeyId;
		static const std::string mMaxAnchorKeyId;

		static const std::string mMinOffsetKeyId;
		static const std::string mMaxOffsetKeyId;

		static const std::string mPivotKeyId;
	};


	const std::string TGridGroupLayoutArchiveKeys::mMinAnchorKeyId = "min_anchor";
	const std::string TGridGroupLayoutArchiveKeys::mMaxAnchorKeyId = "max_anchor";

	const std::string TGridGroupLayoutArchiveKeys::mMinOffsetKeyId = "min_offset";
	const std::string TGridGroupLayoutArchiveKeys::mMaxOffsetKeyId = "max_offset";

	const std::string TGridGroupLayoutArchiveKeys::mPivotKeyId = "pivot";*/


	CGridGroupLayout::CGridGroupLayout() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CGridGroupLayout::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		
		return RC_OK;
	}

	E_RESULT_CODE CGridGroupLayout::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CGridGroupLayout::GetTypeId()));
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CGridGroupLayout::SetCellSize(const TVector2& value)
	{
		if (CMathUtils::IsLessOrEqual(value.x, 0.0f) || CMathUtils::IsLessOrEqual(value.y, 0.0f))
		{
			return RC_INVALID_ARGS;
		}

		mCellSize = value;
		SetDirty(true);

		return RC_OK;
	}

	E_RESULT_CODE CGridGroupLayout::SetSpaceBetweenElements(const TVector2& value)
	{
		mSpaceBetweenElements = value;
		SetDirty(true);
		
		return RC_OK;
	}

	void CGridGroupLayout::SetLeftPadding(F32 value)
	{
		mLeftPadding = value;
		SetDirty(true);
	}

	void CGridGroupLayout::SetRightPadding(F32 value)
	{
		mRightPadding = value;
		SetDirty(true);
	}

	void CGridGroupLayout::SetTopPadding(F32 value)
	{
		mTopPadding = value;
		SetDirty(true);
	}

	void CGridGroupLayout::SetBottomPadding(F32 value)
	{
		mBottomPadding = value;
		SetDirty(true);
	}

	void CGridGroupLayout::SetRowsCount(U16 value)
	{
		mRowsCount = value;
		SetDirty(true);
	}

	void CGridGroupLayout::SetColumnsCount(U16 value)
	{
		mColsCount = value;
		SetDirty(true);
	}

	void CGridGroupLayout::SetDirty(bool value)
	{
		mIsDirty = value;
	}

	const TVector2& CGridGroupLayout::GetCellSize() const
	{
		return mCellSize;
	}

	const TVector2& CGridGroupLayout::GetSpaceBetweenElements() const
	{
		return mSpaceBetweenElements;
	}

	F32 CGridGroupLayout::GetLeftPadding() const
	{
		return mLeftPadding;
	}

	F32 CGridGroupLayout::GetRightPadding() const
	{
		return mRightPadding;
	}

	F32 CGridGroupLayout::GetTopPadding() const
	{
		return mTopPadding;
	}

	F32 CGridGroupLayout::GetBottomPadding() const
	{
		return mBottomPadding;
	}

	U16 CGridGroupLayout::GetRowsCount() const
	{
		return mRowsCount;
	}

	U16 CGridGroupLayout::GetColumnsCount() const
	{
		return mColsCount;
	}

	bool CGridGroupLayout::IsDirty() const
	{
		return mIsDirty;
	}


	IComponent* CreateGridGroupLayout(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CGridGroupLayout, result);
	}


	/*!
		\brief CGridGroupLayoutFactory's definition
	*/

	CGridGroupLayoutFactory::CGridGroupLayoutFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CGridGroupLayoutFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateGridGroupLayout(result);
	}

	E_RESULT_CODE CGridGroupLayoutFactory::SetupComponent(CGridGroupLayout* pComponent, const TGridGroupLayoutParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateGridGroupLayoutFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CGridGroupLayoutFactory, result);
	}
}