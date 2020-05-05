#include "./../../include/scene/components/CDirectionalLight.h"


namespace TDEngine2
{
	CDirectionalLight::CDirectionalLight() :
		CBaseLight()
	{
	}

	E_RESULT_CODE CDirectionalLight::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CDirectionalLight::SetDirection(const TVector3& direction)
	{
		mDirection = Normalize(direction);
		return RC_OK;
	}
	
	const TVector3& CDirectionalLight::GetDirection() const
	{
		return mDirection;
	}


	IComponent* CreateDirectionalLight(const TVector3& direction, E_RESULT_CODE& result)
	{
		CDirectionalLight* pLightInstance = new (std::nothrow) CDirectionalLight();

		if (!pLightInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pLightInstance->Init();

		if (result != RC_OK)
		{
			delete pLightInstance;

			pLightInstance = nullptr;
		}

		pLightInstance->SetDirection(direction);

		return pLightInstance;
	}


	CDirectionalLightFactory::CDirectionalLightFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CDirectionalLightFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CDirectionalLightFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CDirectionalLightFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TDirectionalLightParameters* pSunLightParams = static_cast<const TDirectionalLightParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreateDirectionalLight(pSunLightParams->mDirection, result);
	}

	IComponent* CDirectionalLightFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateDirectionalLight(TVector3(1.0f), result);
	}

	TypeId CDirectionalLightFactory::GetComponentTypeId() const
	{
		return CDirectionalLight::GetTypeId();
	}


	IComponentFactory* CreateDirectionalLightFactory(E_RESULT_CODE& result)
	{
		CDirectionalLightFactory* pDirectionalLightFactoryInstance = new (std::nothrow) CDirectionalLightFactory();

		if (!pDirectionalLightFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pDirectionalLightFactoryInstance->Init();

		if (result != RC_OK)
		{
			delete pDirectionalLightFactoryInstance;

			pDirectionalLightFactoryInstance = nullptr;
		}

		return pDirectionalLightFactoryInstance;
	}
}