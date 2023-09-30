#include "../../../include/scene/components/ShadowMappingComponents.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateShadowCasterComponentFactory)


	CShadowCasterComponent::CShadowCasterComponent() :
		CBaseComponent(), mIsTransparent(false)
	{
	}

	E_RESULT_CODE CShadowCasterComponent::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CShadowCasterComponent::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mIsTransparent = pReader->GetBool("transparent");

		return RC_OK;
	}

	E_RESULT_CODE CShadowCasterComponent::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CShadowCasterComponent::GetTypeId()));
			pWriter->SetBool("transparent", mIsTransparent);
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CShadowCasterComponent::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<CShadowCasterComponent*>(pDestObject))
		{
			pComponent->mIsTransparent = mIsTransparent;

			return RC_OK;
		}

		return RC_FAIL;
	}

	void CShadowCasterComponent::SetTransparentFlag(bool value)
	{
		mIsTransparent = value;
	}

	bool CShadowCasterComponent::IsTransparentObject() const
	{
		return mIsTransparent;
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