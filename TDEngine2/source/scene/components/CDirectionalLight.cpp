#include "../../include/scene/components/CDirectionalLight.h"
#define META_EXPORT_ECS_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateDirectionalLightFactory)
	TDE2_DEFINE_COMPONENT_META(TDirectionalLightComponentData);


	CDirectionalLight::CDirectionalLight() :
		CBaseComponentT()
	{
	}

	const std::string& CDirectionalLight::GetTypeName() const
	{
		static const std::string id{ "directional_light" };
		return id;
	}
	

	IComponent* CreateDirectionalLight(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CDirectionalLight, result);
	}


	/*!
		\brief CDirectionalLightFactory's definition
	*/

	CDirectionalLightFactory::CDirectionalLightFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CDirectionalLightFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateDirectionalLight(result);
	}

	E_RESULT_CODE CDirectionalLightFactory::SetupComponent(CDirectionalLight* pComponent, const TDirectionalLightParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		TDirectionalLightComponentData& dirLightData = pComponent->GetData();
		dirLightData.mColor     = params.mColor;
		dirLightData.mIntensity = params.mIntensity;

		return RC_OK;
	}


	IComponentFactory* CreateDirectionalLightFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CDirectionalLightFactory, result);
	}
}