#include "../../../include/scene/components/CWeatherComponent.h"
#include "../../../include/ecs/CComponentManager.h"
#include <stringUtils.hpp>


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateWeatherComponentFactory)
	TDE2_REGISTER_UNIQUE_COMPONENT(CWeatherComponent)


	struct TWeatherComponentArchiveKeys
	{
		static const std::string mAtmosphereStartRadiusKeyId;
		static const std::string mAtmosphereThicknessKeyId;
		static const std::string mWindDirectionKeyId;
		static const std::string mWindScaleFactorKeyId;
		static const std::string mCoverageKeyId;
		static const std::string mCurlinessKeyId;
		static const std::string mCrispinessKeyId;
		static const std::string mWeatherMapKeyId;
	};

	const std::string TWeatherComponentArchiveKeys::mAtmosphereStartRadiusKeyId = "atmo_start_radius";
	const std::string TWeatherComponentArchiveKeys::mAtmosphereThicknessKeyId = "atmo_thickness";
	const std::string TWeatherComponentArchiveKeys::mWindDirectionKeyId = "wind_direction";
	const std::string TWeatherComponentArchiveKeys::mWindScaleFactorKeyId = "wind_scale";
	const std::string TWeatherComponentArchiveKeys::mCoverageKeyId = "coverage";
	const std::string TWeatherComponentArchiveKeys::mCurlinessKeyId = "curliness";
	const std::string TWeatherComponentArchiveKeys::mCrispinessKeyId = "crispiness";
	const std::string TWeatherComponentArchiveKeys::mWeatherMapKeyId = "weather_texture";


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

		mAtmosphereStartRadius = pReader->GetFloat(TWeatherComponentArchiveKeys::mAtmosphereStartRadiusKeyId, mAtmosphereStartRadius);
		mAtmosphereThickness = pReader->GetFloat(TWeatherComponentArchiveKeys::mAtmosphereThicknessKeyId, mAtmosphereThickness);
		
		pReader->BeginGroup(TWeatherComponentArchiveKeys::mWindDirectionKeyId);
		{
			auto loadWindDirResult = LoadVector2(pReader);
			if (loadWindDirResult.IsOk())
			{
				mWindDirection = loadWindDirResult.Get();
			}
		}
		pReader->EndGroup();

		mWindScaleFactor = pReader->GetFloat(TWeatherComponentArchiveKeys::mWindScaleFactorKeyId, mWindScaleFactor);
		
		mCoverage = pReader->GetFloat(TWeatherComponentArchiveKeys::mCoverageKeyId, mCoverage);
		mCurliness = pReader->GetFloat(TWeatherComponentArchiveKeys::mCurlinessKeyId, mCurliness);
		mCrispiness = pReader->GetFloat(TWeatherComponentArchiveKeys::mCrispinessKeyId, mCrispiness);

		mWeatherMapTextureId = pReader->GetString(TWeatherComponentArchiveKeys::mWeatherMapKeyId);

		return result;
	}

	E_RESULT_CODE CWeatherComponent::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		result = result | pWriter->BeginGroup("component");
		{
			result = result | pWriter->SetUInt32("type_id", static_cast<U32>(CWeatherComponent::GetTypeId()));

			result = result | pWriter->SetFloat(TWeatherComponentArchiveKeys::mAtmosphereStartRadiusKeyId, mAtmosphereStartRadius);
			result = result | pWriter->SetFloat(TWeatherComponentArchiveKeys::mAtmosphereThicknessKeyId, mAtmosphereThickness);

			result = result | pWriter->BeginGroup(TWeatherComponentArchiveKeys::mWindDirectionKeyId);
			result = result | SaveVector2(pWriter, mWindDirection);
			result = result | pWriter->EndGroup();

			result = result | pWriter->SetFloat(TWeatherComponentArchiveKeys::mWindScaleFactorKeyId, mWindScaleFactor);

			result = result | pWriter->SetFloat(TWeatherComponentArchiveKeys::mCoverageKeyId, mCoverage);
			result = result | pWriter->SetFloat(TWeatherComponentArchiveKeys::mCurlinessKeyId, mCurliness);
			result = result | pWriter->SetFloat(TWeatherComponentArchiveKeys::mCrispinessKeyId, mCrispiness);

			result = result | pWriter->SetString(TWeatherComponentArchiveKeys::mWeatherMapKeyId, mWeatherMapTextureId);
		}
		result = result | pWriter->EndGroup();

		return result;
	}

	E_RESULT_CODE CWeatherComponent::Clone(IComponent*& pDestObject) const
	{
		if (auto pSourceComponent = dynamic_cast<CWeatherComponent*>(pDestObject))
		{
			pSourceComponent->mAtmosphereStartRadius = mAtmosphereStartRadius;
			pSourceComponent->mAtmosphereThickness = mAtmosphereThickness;
			pSourceComponent->mWindDirection = mWindDirection;
			pSourceComponent->mWindScaleFactor = mWindScaleFactor;

			pSourceComponent->mCoverage = mCoverage;
			pSourceComponent->mCurliness = mCurliness;
			pSourceComponent->mCrispiness = mCrispiness;

			pSourceComponent->mWeatherMapTextureId = mWeatherMapTextureId;

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