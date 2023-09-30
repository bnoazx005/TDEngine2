#include "../../../include/graphics/UI/CUISliderComponent.h"
#include "../../../include/math/MathUtils.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateUISliderFactory)


	CUISlider::CUISlider() :
		CBaseComponent()
	{
	}


	struct TUISliderArchiveKeys
	{
		static const std::string mMarkerEntityRefKeyId;
		static const std::string mValueKeyId;
		static const std::string mMinValueKeyId;
		static const std::string mMaxValueKeyId;
	};


	const std::string TUISliderArchiveKeys::mMarkerEntityRefKeyId = "marker_entity_ref";
	const std::string TUISliderArchiveKeys::mValueKeyId = "value";
	const std::string TUISliderArchiveKeys::mMinValueKeyId = "min_value";
	const std::string TUISliderArchiveKeys::mMaxValueKeyId = "max_value";

	E_RESULT_CODE CUISlider::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mMarkerEntityRef = static_cast<TEntityId>(pReader->GetUInt32(TUISliderArchiveKeys::mMarkerEntityRefKeyId));
		
		mMinValue = pReader->GetFloat(TUISliderArchiveKeys::mMinValueKeyId);
		mMaxValue = pReader->GetFloat(TUISliderArchiveKeys::mMaxValueKeyId);

		SetValue(pReader->GetFloat(TUISliderArchiveKeys::mValueKeyId));

		return RC_OK;
	}

	E_RESULT_CODE CUISlider::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CUISlider::GetTypeId()));

			pWriter->SetUInt32(TUISliderArchiveKeys::mMarkerEntityRefKeyId, static_cast<U32>(mMarkerEntityRef));
			
			pWriter->SetFloat(TUISliderArchiveKeys::mMinValueKeyId, mMinValue);
			pWriter->SetFloat(TUISliderArchiveKeys::mMaxValueKeyId, mMaxValue);
			pWriter->SetFloat(TUISliderArchiveKeys::mValueKeyId, mValue);
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CUISlider::PostLoad(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesIdentifiersRemapper)
	{
		mMarkerEntityRef = entitiesIdentifiersRemapper.Resolve(mMarkerEntityRef);

		return CBaseComponent::PostLoad(pEntityManager, entitiesIdentifiersRemapper);
	}

	E_RESULT_CODE CUISlider::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<CUISlider*>(pDestObject))
		{
			pComponent->mMarkerEntityRef = mMarkerEntityRef;
			pComponent->mValue = mValue;
			pComponent->mMinValue = mMinValue;
			pComponent->mMaxValue = mMaxValue;

			return RC_OK;
		}

		return RC_FAIL;
	}

	void CUISlider::SetValue(F32 value)
	{
		mValue = CMathUtils::Clamp(mMinValue, mMaxValue, value);
	}

	void CUISlider::SetMinValue(F32 value)
	{
		mMinValue = value;
	}

	void CUISlider::SetMaxValue(F32 value)
	{
		mMaxValue = value;
	}

	void CUISlider::SetMarkerEntityId(TEntityId markerId)
	{
		mMarkerEntityRef = markerId;
	}

	F32 CUISlider::GetValue() const
	{
		return mValue;
	}

	F32 CUISlider::GetMinValue() const
	{
		return mMinValue;
	}

	F32 CUISlider::GetMaxValue() const
	{
		return mMaxValue;
	}

	TEntityId CUISlider::GetMarkerEntityId() const
	{
		return mMarkerEntityRef;
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