#include "../../include/scene/components/CDirectionalLight.h"


namespace TDEngine2
{
	CDirectionalLight::CDirectionalLight() :
		CBaseLight()
	{
	}

	E_RESULT_CODE CDirectionalLight::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		pReader->BeginGroup("direction");
		{
			if (auto directionResult = LoadVector3(pReader))
			{
				//mDirection = Normalize(directionResult.Get());
			}
		}
		pReader->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CDirectionalLight::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CDirectionalLight::GetTypeId()));
			pWriter->BeginGroup("direction");
			{
				//SaveVector3(pWriter, mDirection);
			}
			pWriter->EndGroup();
		}
		pWriter->EndGroup();

		return RC_OK;
	}
	

	IComponent* CreateDirectionalLight(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CDirectionalLight, result);
	}


	/*!
		\brief CDirectionalLightFactory's definition
	*/

	CDirectionalLightFactory::CDirectionalLightFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CDirectionalLightFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateDirectionalLight(result);
	}

	E_RESULT_CODE CDirectionalLightFactory::SetupComponent(CDirectionalLight* pComponent, const TDirectionalLightParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		pComponent->SetColor(params.mColor);
		pComponent->SetIntensity(params.mIntensity);

		return RC_OK;
	}


	IComponentFactory* CreateDirectionalLightFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CDirectionalLightFactory, result);
	}
}