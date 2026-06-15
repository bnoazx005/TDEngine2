#include "../../../include/scene/components/CWeatherComponent.h"
#include "../../../include/ecs/CComponentManager.h"
#include <stringUtils.hpp>
#define META_EXPORT_ECS_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateWeatherComponentFactory)
	TDE2_REGISTER_UNIQUE_COMPONENT(CWeatherComponent)
	TDE2_DEFINE_COMPONENT_META(TWeatherComponentData)


	CWeatherComponent::CWeatherComponent() :
		CBaseComponentT()
	{
	}

	const std::string& CWeatherComponent::GetTypeName() const
	{
		static const std::string typeName = "weather_config";
		return typeName;
	}


	IComponent* CreateWeatherComponent(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CWeatherComponent, result);
	}


	/*!
		\brief CWeatherComponentFactory's definition
	*/

	CWeatherComponentFactory::CWeatherComponentFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CWeatherComponentFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateWeatherComponent(result);
	}

	E_RESULT_CODE CWeatherComponentFactory::SetupComponent(CWeatherComponent* pComponent, const TWeatherComponentParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateWeatherComponentFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CWeatherComponentFactory, result);
	}
}