#include "../../include/scene/components/CPointLight.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreatePointLightFactory)


	CPointLight::CPointLight() :
		CBaseLight()
	{
	}

	E_RESULT_CODE CPointLight::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mRange = pReader->GetFloat("range");

		return RC_OK;
	}

	E_RESULT_CODE CPointLight::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CPointLight::GetTypeId()));
			pWriter->SetFloat("range", mRange);
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CPointLight::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<CPointLight*>(pDestObject))
		{
			pComponent->mColor = mColor;
			pComponent->mIntensity = mIntensity;
			pComponent->mRange = mRange;

			return RC_OK;
		}

		return RC_FAIL;
	}

	E_RESULT_CODE CPointLight::SetRange(F32 range)
	{
		if (range < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		mRange = range;

		return RC_OK;
	}

	F32 CPointLight::GetRange() const
	{
		return mRange;
	}
	

	IComponent* CreatePointLight(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CPointLight, result);
	}


	/*!
		\brief CPointLightFactory's definition
	*/

	CPointLightFactory::CPointLightFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CPointLightFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreatePointLight(result);
	}

	E_RESULT_CODE CPointLightFactory::SetupComponent(CPointLight* pComponent, const TPointLightParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		pComponent->SetColor(params.mColor);
		pComponent->SetIntensity(params.mIntensity);
		pComponent->SetRange(params.mRange);

		return RC_OK;
	}


	IComponentFactory* CreatePointLightFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CPointLightFactory, result);
	}
}