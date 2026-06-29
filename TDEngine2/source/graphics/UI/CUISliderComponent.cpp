#include "../../../include/graphics/UI/CUISliderComponent.h"
#include "../../../include/math/MathUtils.h"
#define META_EXPORT_ECS_SECTION
#include "../../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateUISliderFactory)
	TDE2_DEFINE_COMPONENT_META(TUISliderComponentData)


	CUISlider::CUISlider() :
		CBaseComponentT()
	{
	}

	E_RESULT_CODE CUISlider::PostLoad(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesIdentifiersRemapper)
	{
		mData.mMarkerEntityRef = entitiesIdentifiersRemapper.Resolve(mData.mMarkerEntityRef);

		return CBaseComponentT::PostLoad(pEntityManager, entitiesIdentifiersRemapper);
	}

	void CUISlider::SetValue(F32 value)
	{
		mData.mValue = CMathUtils::Clamp(mData.mMinValue, mData.mMaxValue, value);
	}

	void CUISlider::SetMinValue(F32 value)
	{
		mData.mMinValue = value;
	}

	void CUISlider::SetMaxValue(F32 value)
	{
		mData.mMaxValue = value;
	}

	void CUISlider::SetMarkerEntityId(TEntityId markerId)
	{
		mData.mMarkerEntityRef = markerId;
	}

	F32 CUISlider::GetValue() const
	{
		return mData.mValue;
	}

	F32 CUISlider::GetMinValue() const
	{
		return mData.mMinValue;
	}

	F32 CUISlider::GetMaxValue() const
	{
		return mData.mMaxValue;
	}

	TEntityId CUISlider::GetMarkerEntityId() const
	{
		return mData.mMarkerEntityRef;
	}


	IComponent* CreateUISlider(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CUISlider, result);
	}


	/*!
		\brief CUISliderFactory's definition
	*/

	CUISliderFactory::CUISliderFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CUISliderFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateUISlider(result);
	}

	E_RESULT_CODE CUISliderFactory::SetupComponent(CUISlider* pComponent, const TUISliderParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateUISliderFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CUISliderFactory, result);
	}
}