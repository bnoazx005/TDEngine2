/*
	Clouds rendering based on implementation described at
	https://www.guerrilla-games.com/read/the-real-time-volumetric-cloudscapes-of-horizon-zero-dawn
*/

#include "../../include/ecs/CWeatherSystem.h"
#include "../../include/ecs/CSystemManager.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IJobManager.h"
#include "../../include/core/CProjectSettings.h"
#include "../../include/graphics/CBaseTexture3D.h"
#include "../../include/graphics/IMaterial.h"
#include "../../include/graphics/IShader.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/IPipeline.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/math/MathUtils.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/scene/components/CWeatherComponent.h"
#include "stringUtils.hpp"
#include "randomUtils.hpp"
#include <vector>


namespace TDEngine2
{
	CWeatherSystem::CWeatherSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CWeatherSystem::Init(const TWeatherSystemInitParams& params)
	{
		TDE2_PROFILER_SCOPE("CWeatherSystem::Init");

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!params.mpResourceManager || !params.mpGraphicsContext || !params.mpJobManager)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = params.mpResourceManager;
		mpGraphicsContext = params.mpGraphicsContext;
		mpJobManager = params.mpJobManager;

		mIsInitialized = true;

		return RC_OK;
	}

	void CWeatherSystem::InjectBindings(IWorld* pWorld)
	{
		auto&& entities = pWorld->FindEntitiesWithComponents<CWeatherComponent>();
		if (entities.empty())
		{
			return;
		}

		CEntity* pEntity = pWorld->FindEntity(entities.front());
		if (!pEntity)
		{
			return;
		}

		mpWeatherComponent = pEntity->GetComponent<CWeatherComponent>();
	}


	TDE2_STATIC_CONSTEXPR F32 EarthRadius = 600000.0f;


	void CWeatherSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CWeatherSystem::Update");

		if (!mpWeatherComponent)
		{
			return;
		}

		struct
		{
			TVector4  mAtmosphereParameters; // x - Earth's radius, y - inner atmosphere's radius, z - thickness
			TVector4  mWindParameters; // xy - direction, w - scale factor
			TColor32F mAmbientCloudsColor;
			F32       mAbsorption;
			F32       mCoverage;
			F32       mCurliness;
			F32       mCrispiness;
			F32       mDensityFactor;
		} uniformsData;

		const TWeatherComponentData& weatherData = mpWeatherComponent->GetData();

		uniformsData.mAmbientCloudsColor   = weatherData.mAmbientCloudColor;
		uniformsData.mAtmosphereParameters = TVector4{ EarthRadius, weatherData.mAtmosphereStartRadius, weatherData.mAtmosphereThickness, 0.0f };
		uniformsData.mWindParameters       = TVector4{ weatherData.mWindDirection.x, weatherData.mWindDirection.y, 0.0f, weatherData.mWindScaleFactor };
		uniformsData.mAbsorption           = weatherData.mSunLightAbsorption;
		uniformsData.mCoverage             = weatherData.mCoverage;
		uniformsData.mCurliness            = weatherData.mCurliness;
		uniformsData.mCrispiness           = weatherData.mCrispiness;
		uniformsData.mDensityFactor        = weatherData.mDensityFactor;

		const TResourceId volumetricCloudsShaderHandle = mpResourceManager->Load<IShader>(CProjectSettings::Get()->mGraphicsSettings.mVolumetricCloudsMainShader);
		TDE2_ASSERT(TResourceId::Invalid != volumetricCloudsShaderHandle);

		if (auto pVolumetricCloudsMainShader = mpResourceManager->GetResource<IShader>(volumetricCloudsShaderHandle))
		{
			pVolumetricCloudsMainShader->SetUserUniformsBuffer(1, reinterpret_cast<const U8*>(&uniformsData), sizeof(uniformsData));

			auto pWeatherMapTexture = mpResourceManager->GetResource<ITexture2D>(mpResourceManager->Load<ITexture2D>(weatherData.mWeatherMapTextureId));
			pVolumetricCloudsMainShader->SetTextureResource("WeatherMap", pWeatherMapTexture.Get());
		}
	}


	TDE2_API ISystem* CreateWeatherSystem(const TWeatherSystemInitParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CWeatherSystem, result, params);
	}


	struct TWeatherSystemAutoInitializer : TSystemAutoInitializer
	{
		virtual ~TWeatherSystemAutoInitializer() = default;

		ISystem* GetSystem(IWorld* pWorld, const TRequestSubsystemCallback& subsystemsProviderCallback)
		{
			E_RESULT_CODE result = RC_OK;
			return CreateWeatherSystem(
				{
					PolymorphicCast<IResourceManager*>(subsystemsProviderCallback(E_ENGINE_SUBSYSTEM_TYPE::EST_RESOURCE_MANAGER)),
					PolymorphicCast<IGraphicsContext*>(subsystemsProviderCallback(E_ENGINE_SUBSYSTEM_TYPE::EST_GRAPHICS_CONTEXT)),
					PolymorphicCast<IJobManager*>(subsystemsProviderCallback(E_ENGINE_SUBSYSTEM_TYPE::EST_JOB_MANAGER))
				}, result);
		}
	};


	TDE2_REGISTER_SYSTEM(TWeatherSystemAutoInitializer);
}