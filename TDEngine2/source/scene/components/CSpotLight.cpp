#include "../../include/scene/components/CSpotLight.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateSpotLightFactory)


	CSpotLight::CSpotLight() :
		CBaseLight()
	{
	}

	E_RESULT_CODE CSpotLight::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mConeAngle = pReader->GetFloat("angle");
		mRange = pReader->GetFloat("range");

		return RC_OK;
	}

	E_RESULT_CODE CSpotLight::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CSpotLight::GetTypeId()));
			pWriter->SetFloat("angle", mConeAngle);
			pWriter->SetFloat("range", mRange);
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CSpotLight::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<CSpotLight*>(pDestObject))
		{
			pComponent->mColor = mColor;
			pComponent->mIntensity = mIntensity;
			pComponent->mConeAngle = mConeAngle;
			pComponent->mRange = mRange;

			return RC_OK;
		}

		return RC_FAIL;
	}

	E_RESULT_CODE CSpotLight::SetAngle(F32 value)
	{
		mConeAngle = value;
		return RC_OK;
	}

	F32 CSpotLight::GetAngle() const
	{
		return mConeAngle;
	}

	E_RESULT_CODE CSpotLight::SetRange(F32 range)
	{
		if (range < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		mRange = range;

		return RC_OK;
	}

	F32 CSpotLight::GetRange() const
	{
		return mRange;
	}


	IComponent* CreateSpotLight(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CSpotLight, result);
	}


	/*!
		\brief CSpotLightFactory's definition
	*/

	CSpotLightFactory::CSpotLightFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CSpotLightFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateSpotLight(result);
	}

	E_RESULT_CODE CSpotLightFactory::SetupComponent(CSpotLight* pComponent, const TSpotLightParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		pComponent->SetColor(params.mColor);
		pComponent->SetIntensity(params.mIntensity);
		pComponent->SetAngle(params.mAngle);
		pComponent->SetRange(params.mRange);

		return RC_OK;
	}


	IComponentFactory* CreateSpotLightFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CSpotLightFactory, result);
	}
}