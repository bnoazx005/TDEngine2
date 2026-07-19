#include "../../include/scene/components/CPointLight.h"
#define META_EXPORT_ECS_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreatePointLightFactory)
	TDE2_DEFINE_COMPONENT_META(TPointLightComponentData);


	CPointLight::CPointLight() :
		CBaseComponentT()
	{
	}

	const std::string& CPointLight::GetTypeName() const
	{
		static const std::string id{ "point_light" };
		return id;
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

		TPointLightComponentData& pointLightData = pComponent->GetData();
		pointLightData.mColor     = params.mColor;
		pointLightData.mIntensity = params.mIntensity;
		pointLightData.mRange     = params.mRange;

		return RC_OK;
	}


	IComponentFactory* CreatePointLightFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CPointLightFactory, result);
	}
}