#include "../../../include/scene/components/CSplashScreenItemComponent.h"
#include "../../../include/ecs/CComponentManager.h"
#include <stringUtils.hpp>
#define META_EXPORT_ECS_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateSplashScreenItemComponentFactory)
	TDE2_DEFINE_COMPONENT_META(TSplashScreenComponentData)


	CSplashScreenItemComponent::CSplashScreenItemComponent() :
		CBaseComponentT()
	{
	}

	const std::string& CSplashScreenItemComponent::GetTypeName() const
	{
		static const std::string typeName = "splash_screen_item";
		return typeName;
	}


	IComponent* CreateSplashScreenItemComponent(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CSplashScreenItemComponent, result);
	}


	/*!
		\brief CSplashScreenItemComponentFactory's definition
	*/

	CSplashScreenItemComponentFactory::CSplashScreenItemComponentFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CSplashScreenItemComponentFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateSplashScreenItemComponent(result);
	}

	E_RESULT_CODE CSplashScreenItemComponentFactory::SetupComponent(CSplashScreenItemComponent* pComponent, const TSplashScreenItemComponentParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateSplashScreenItemComponentFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CSplashScreenItemComponentFactory, result);
	}


	TDE2_DEFINE_FLAG_COMPONENT(SplashScreenContainerRoot);
}