#include "../../../include/graphics/UI/GroupLayoutComponents.h"
#include "../../../include/math/MathUtils.h"
#define META_EXPORT_UI_SECTION
#define META_EXPORT_ECS_SECTION
#include "../../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateGridGroupLayoutFactory)
	TDE2_DEFINE_COMPONENT_META(TGridGroupLayoutComponentData)


	CGridGroupLayout::CGridGroupLayout() :
		CBaseComponentT()
	{
	}

	E_RESULT_CODE CGridGroupLayout::SetCellSize(const TVector2& value)
	{
		if (CMathUtils::IsLessOrEqual(value.x, 0.0f) || CMathUtils::IsLessOrEqual(value.y, 0.0f))
		{
			return RC_INVALID_ARGS;
		}

		mData.mCellSize = value;
		SetDirty(true);

		return RC_OK;
	}

	E_RESULT_CODE CGridGroupLayout::SetSpaceBetweenElements(const TVector2& value)
	{
		mData.mSpaceBetweenElements = value;
		SetDirty(true);
		
		return RC_OK;
	}

	void CGridGroupLayout::SetElementsAlignType(E_UI_ELEMENT_ALIGNMENT_TYPE value)
	{
		mData.mAlignType = value;
		SetDirty(true);
	}

	void CGridGroupLayout::SetDirty(bool value)
	{
		mData.mIsDirty = value;
	}

	const TVector2& CGridGroupLayout::GetCellSize() const
	{
		return mData.mCellSize;
	}

	const TVector2& CGridGroupLayout::GetSpaceBetweenElements() const
	{
		return mData.mSpaceBetweenElements;
	}

	E_UI_ELEMENT_ALIGNMENT_TYPE CGridGroupLayout::GetElementsAlignType() const
	{
		return mData.mAlignType;
	}

	bool CGridGroupLayout::IsDirty() const
	{
		return mData.mIsDirty;
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