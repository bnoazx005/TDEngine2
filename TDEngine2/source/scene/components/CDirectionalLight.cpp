#include "../../include/scene/components/CDirectionalLight.h"


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
		return CREATE_IMPL(IComponent, CDirectionalLight, result);
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
		return CREATE_IMPL(IComponentFactory, CDirectionalLightFactory, result);
	}
}