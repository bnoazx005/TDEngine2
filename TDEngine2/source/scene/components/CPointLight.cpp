#include "../../include/scene/components/CPointLight.h"


namespace TDEngine2
{
	CPointLight::CPointLight() :
		CBaseLight()
	{
	}

	E_RESULT_CODE CPointLight::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CPointLight::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mRange = pReader->GetFloat("range");

		return RC_OK;
	}

	E_RESULT_CODE CPointLight::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CPointLight::GetTypeId()));
			pWriter->SetFloat("range", mRange);
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CPointLight::SetRange(F32 range)
	{
		if (range < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		mRange = range;

		return RC_OK;
	}

	F32 CPointLight::GetRange() const
	{
		return mRange;
	}
	

	IComponent* CreatePointLight(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CPointLight, result);
	}


	CPointLightFactory::CPointLightFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CPointLightFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CPointLightFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CPointLightFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TPointLightParameters* pPointLightParams = static_cast<const TPointLightParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreatePointLight(result);
	}

	IComponent* CPointLightFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreatePointLight(result);
	}

	TypeId CPointLightFactory::GetComponentTypeId() const
	{
		return CPointLight::GetTypeId();
	}


	IComponentFactory* CreatePointLightFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CPointLightFactory, result);
	}
}