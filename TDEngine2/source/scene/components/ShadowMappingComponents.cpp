#include "../../../include/scene/components/ShadowMappingComponents.h"
#define META_EXPORT_ECS_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateShadowCasterComponentFactory)
	TDE2_DEFINE_COMPONENT_META(TShadowCasterComponentData)


	CShadowCasterComponent::CShadowCasterComponent() :
		CBaseComponentT()
	{
	}

	void CShadowCasterComponent::SetTransparentFlag(bool value)
	{
		mData.mIsTransparent = value;
	}

	bool CShadowCasterComponent::IsTransparentObject() const
	{
		return mData.mIsTransparent;
	}

	const std::string& CShadowCasterComponent::GetTypeName() const
	{
		static std::string componentName{ "shadow_caster" };
		return componentName;
	}


	IComponent* CreateShadowCasterComponent(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CShadowCasterComponent, result);
	}


	/*!
		\brief CShadowCasterComponentFactory's definition
	*/

	CShadowCasterComponentFactory::CShadowCasterComponentFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CShadowCasterComponentFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateShadowCasterComponent(result);
	}

	E_RESULT_CODE CShadowCasterComponentFactory::SetupComponent(CShadowCasterComponent* pComponent, const TShadowCasterComponentParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		pComponent->SetTransparentFlag(params.mIsTransparent);

		return RC_OK;
	}


	IComponentFactory* CreateShadowCasterComponentFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CShadowCasterComponentFactory, result);
	}


	TDE2_DEFINE_FLAG_COMPONENT(ShadowReceiverComponent);
	TDE2_DEFINE_FLAG_COMPONENT(SkyboxComponent);
}