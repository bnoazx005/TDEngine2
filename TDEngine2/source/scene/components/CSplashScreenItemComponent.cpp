#include "../../../include/scene/components/CSplashScreenItemComponent.h"
#include "../../../include/ecs/CComponentManager.h"
#include <stringUtils.hpp>


namespace TDEngine2
{
	struct TSplashScreenItemArchiveKeys
	{
		static const std::string mDurationKeyId;
	};


	const std::string TSplashScreenItemArchiveKeys::mDurationKeyId = "duration";



	TDE2_REGISTER_COMPONENT_FACTORY(CreateSplashScreenItemComponentFactory)


	CSplashScreenItemComponent::CSplashScreenItemComponent() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CSplashScreenItemComponent::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mDuration = pReader->GetFloat(TSplashScreenItemArchiveKeys::mDurationKeyId, mDuration);

		return RC_OK;
	}

	E_RESULT_CODE CSplashScreenItemComponent::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CSplashScreenItemComponent::GetTypeId()));
			
			pWriter->SetFloat(TSplashScreenItemArchiveKeys::mDurationKeyId, mDuration);

		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CSplashScreenItemComponent::Clone(IComponent*& pDestObject) const
	{
		if (auto pSourceComponent = dynamic_cast<CSplashScreenItemComponent*>(pDestObject))
		{
			pSourceComponent->mDuration = mDuration;

			return RC_OK;
		}

		return RC_FAIL;
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