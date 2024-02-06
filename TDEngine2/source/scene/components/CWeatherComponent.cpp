#include "../../../include/scene/components/CWeatherComponent.h"
#include "../../../include/ecs/CComponentManager.h"
#include <stringUtils.hpp>


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateWeatherComponentFactory)
	TDE2_REGISTER_UNIQUE_COMPONENT(CWeatherComponent)


	CWeatherComponent::CWeatherComponent() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CWeatherComponent::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		return result;
	}

	E_RESULT_CODE CWeatherComponent::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CWeatherComponent::GetTypeId()));

		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CWeatherComponent::Clone(IComponent*& pDestObject) const
	{
		if (auto pSourceComponent = dynamic_cast<CWeatherComponent*>(pDestObject))
		{
			return RC_OK;
		}

		return RC_FAIL;
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