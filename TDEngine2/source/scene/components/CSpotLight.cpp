#include "../../include/scene/components/CSpotLight.h"
#define META_EXPORT_ECS_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateSpotLightFactory)
	TDE2_DEFINE_COMPONENT_META(TSpotLightComponentData);


	CSpotLight::CSpotLight() :
		CBaseComponentT()
	{
	}

	const std::string& CSpotLight::GetTypeName() const
	{
		static const std::string id{ "spot_light" };
		return id;
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

		TSpotLightComponentData& spotLightData = pComponent->GetData();

		spotLightData.mColor     = params.mColor;
		spotLightData.mIntensity = params.mIntensity;
		spotLightData.mConeAngle = params.mAngle;
		spotLightData.mRange     = params.mRange;

		return RC_OK;
	}


	IComponentFactory* CreateSpotLightFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CSpotLightFactory, result);
	}
}