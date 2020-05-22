#include "../../../include/scene/components/ShadowMappingComponents.h"


namespace TDEngine2
{
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
	
	void CShadowCasterComponent::SetTransparentFlag(bool value)
	{
		mIsTransparent = value;
	}

	bool CShadowCasterComponent::IsTransparentObject() const
	{
		return mIsTransparent;
	}


	IComponent* CreateShadowCasterComponent(E_RESULT_CODE& result)
	{
		CShadowCasterComponent* pShadowCasterComponentInstance = new (std::nothrow) CShadowCasterComponent();

		if (!pShadowCasterComponentInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pShadowCasterComponentInstance->Init();

		if (result != RC_OK)
		{
			delete pShadowCasterComponentInstance;

			pShadowCasterComponentInstance = nullptr;
		}

		return pShadowCasterComponentInstance;
	}


	CShadowCasterComponentFactory::CShadowCasterComponentFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CShadowCasterComponentFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CShadowCasterComponentFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CShadowCasterComponentFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		E_RESULT_CODE result = RC_OK;

		CShadowCasterComponent* pShadowCaster = dynamic_cast<CShadowCasterComponent*>(CreateShadowCasterComponent(result));
		if (pShadowCaster)
		{
			if (const TShadowCasterComponentParameters* params = static_cast<const TShadowCasterComponentParameters*>(pParams))
			{
				pShadowCaster->SetTransparentFlag(params->mIsTransparent);
			}
		}

		return pShadowCaster;
	}

	IComponent* CShadowCasterComponentFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateShadowCasterComponent(result);
	}

	TypeId CShadowCasterComponentFactory::GetComponentTypeId() const
	{
		return CShadowCasterComponent::GetTypeId();
	}


	IComponentFactory* CreateShadowCasterComponentFactory(E_RESULT_CODE& result)
	{
		CShadowCasterComponentFactory* pShadowCasterComponentFactoryInstance = new (std::nothrow) CShadowCasterComponentFactory();

		if (!pShadowCasterComponentFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pShadowCasterComponentFactoryInstance->Init();

		if (result != RC_OK)
		{
			delete pShadowCasterComponentFactoryInstance;

			pShadowCasterComponentFactoryInstance = nullptr;
		}

		return pShadowCasterComponentFactoryInstance;
	}


	TDE2_DEFINE_FLAG_COMPONENT(ShadowReceiverComponent);
}