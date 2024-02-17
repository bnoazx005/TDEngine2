/*
	Clouds rendering based on implementation described at
	https://www.guerrilla-games.com/read/the-real-time-volumetric-cloudscapes-of-horizon-zero-dawn
*/

#include "../../include/ecs/CWeatherSystem.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IJobManager.h"
#include "../../include/core/CProjectSettings.h"
#include "../../include/graphics/CBaseTexture3D.h"
#include "../../include/graphics/IMaterial.h"
#include "../../include/graphics/IShader.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/math/MathUtils.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/scene/components/CWeatherComponent.h"
#include "stringUtils.hpp"
#include "randomUtils.hpp"
#include <vector>


namespace TDEngine2
{
	static const std::string LowFreqCloudsNoise3DTextureId  = "CloudsLowFreqNoise";
	static const std::string HighFreqCloudsNoise3DTextureId = "CloudsHighFreqNoise";

	static constexpr U32 LowFreqCloudsNoiseTextureSize  = 128;
	static constexpr U32 HighFreqCloudsNoiseTextureSize = 32;
	static constexpr F32 InvLowFreqCloudsNoiseTextureSize = 1.0f / LowFreqCloudsNoiseTextureSize;
	static constexpr F32 InvHighFreqCloudsNoiseTextureSize = 1.0f / HighFreqCloudsNoiseTextureSize;


	CWeatherSystem::CWeatherSystem() :
		CBaseSystem()
	{
	}


	static void GenerateCloudsNoiseTexture(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IJobManager* pJobManager,
		TResourceId lowFreqCloudsNoiseTexHandle, TResourceId highFreqCloudsNoiseTexHandle)
	{
		TDE2_PROFILER_SCOPE("CWeatherSystem::GenerateCloudsNoiseTexture");

		// \note Generate low frequency clouds noise
		pJobManager->SubmitJob(nullptr, [pGraphicsContext, pResourceManager, lowFreqCloudsNoiseTexHandle](auto)
		{
			LOG_MESSAGE("[GenerateCloudsNoiseTexture] Start generate low frequency clouds noise 3D texture...");
			auto pLowFreqCloudsNoiseGenerationShader = pResourceManager->GetResource<IShader>(pResourceManager->Load<IShader>("Shaders/Default/GenerateLowFreqCloudsNoise.cshader"));
			pLowFreqCloudsNoiseGenerationShader->SetTextureResource("noiseTexture", pResourceManager->GetResource<ITexture>(lowFreqCloudsNoiseTexHandle).Get());
			pLowFreqCloudsNoiseGenerationShader->Bind();

			pGraphicsContext->DispatchCompute(LowFreqCloudsNoiseTextureSize / 8, LowFreqCloudsNoiseTextureSize / 8, LowFreqCloudsNoiseTextureSize / 8);

			LOG_MESSAGE("[GenerateCloudsNoiseTexture] Generation LowFreqNoise finished");
		}, { E_JOB_PRIORITY_TYPE::NORMAL, false, "GenerateLowFreqCloudsNoiseJob" });

		// \note Generate high frequency clouds noise
		pJobManager->SubmitJob(nullptr, [pGraphicsContext, pResourceManager, highFreqCloudsNoiseTexHandle](auto)
		{
			LOG_MESSAGE("[GenerateCloudsNoiseTexture] Start generate high frequency clouds noise 3D texture...");

			auto pHighFreqCloudsNoiseGenerationShader = pResourceManager->GetResource<IShader>(pResourceManager->Load<IShader>("Shaders/Default/GenerateHiFreqCloudsNoise.cshader"));
			pHighFreqCloudsNoiseGenerationShader->SetTextureResource("noiseTexture", pResourceManager->GetResource<ITexture>(highFreqCloudsNoiseTexHandle).Get());
			pHighFreqCloudsNoiseGenerationShader->Bind();

			pGraphicsContext->DispatchCompute(HighFreqCloudsNoiseTextureSize, HighFreqCloudsNoiseTextureSize, HighFreqCloudsNoiseTextureSize);

			LOG_MESSAGE("[GenerateCloudsNoiseTexture] Generation HiFreqNoise finished");
		}, { E_JOB_PRIORITY_TYPE::NORMAL, false, "GenerateHighFreqCloudsNoiseJob" });
	}


	static E_RESULT_CODE CreateCloudsNoiseTexture(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IJobManager* pJobManager)
	{
		TTexture3DParameters lowFreqCloudsNoiseTextureParams 
		{ 
			LowFreqCloudsNoiseTextureSize, 
			LowFreqCloudsNoiseTextureSize, 
			LowFreqCloudsNoiseTextureSize, 
			FT_NORM_UBYTE4, 1, 1, 0, 
		};
		lowFreqCloudsNoiseTextureParams.mIsWriteable = true;

		TTexture3DParameters highFreqCloudsNoiseTextureParams
		{ 
			HighFreqCloudsNoiseTextureSize,
			HighFreqCloudsNoiseTextureSize, 
			HighFreqCloudsNoiseTextureSize, 
			FT_NORM_UBYTE4, 1, 1, 0
		};
		highFreqCloudsNoiseTextureParams.mIsWriteable = true;

		const TResourceId lowFreqCloudsTextureHandle = pResourceManager->Create<ITexture3D>(LowFreqCloudsNoise3DTextureId, lowFreqCloudsNoiseTextureParams);
		const TResourceId highFreqCloudsTextureHandle = pResourceManager->Create<ITexture3D>(HighFreqCloudsNoise3DTextureId, highFreqCloudsNoiseTextureParams);

		const std::array<TResourceId, 2> noiseTextures { lowFreqCloudsTextureHandle, highFreqCloudsTextureHandle };
		for (auto&& currTextureHandle : noiseTextures)
		{
			auto pTexture = pResourceManager->GetResource<ITexture3D>(currTextureHandle);
			if (!pTexture)
			{
				continue;
			}

			pTexture->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);
			pTexture->SetUWrapMode(E_ADDRESS_MODE_TYPE::AMT_WRAP);
			pTexture->SetVWrapMode(E_ADDRESS_MODE_TYPE::AMT_WRAP);
			pTexture->SetWWrapMode(E_ADDRESS_MODE_TYPE::AMT_WRAP);
		}

		const TResourceId volumetricCloudsShaderHandle = pResourceManager->Load<IMaterial>(CProjectSettings::Get()->mGraphicsSettings.mVolumetricCloudsMainShader);
		TDE2_ASSERT(TResourceId::Invalid != volumetricCloudsShaderHandle);
		
		if (auto pVolumetricCloudsMainShader = pResourceManager->GetResource<IShader>(volumetricCloudsShaderHandle))
		{
			pVolumetricCloudsMainShader->SetTextureResource("LowFreqCloudsNoiseTex", pResourceManager->GetResource<ITexture>(lowFreqCloudsTextureHandle).Get());
			pVolumetricCloudsMainShader->SetTextureResource("HiFreqCloudsNoiseTex", pResourceManager->GetResource<ITexture>(highFreqCloudsTextureHandle).Get());
		}

		TDE2_ASSERT(TResourceId::Invalid != lowFreqCloudsTextureHandle);
		TDE2_ASSERT(TResourceId::Invalid != highFreqCloudsTextureHandle);

		pJobManager->SubmitJob(nullptr, [=](const TJobArgs& args)
		{
			GenerateCloudsNoiseTexture(pResourceManager, pGraphicsContext, pJobManager, lowFreqCloudsTextureHandle, highFreqCloudsTextureHandle);
		}, { E_JOB_PRIORITY_TYPE::NORMAL, false, "GenerateCloudTexturesJob" });

		return RC_OK;
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

		E_RESULT_CODE result = CreateCloudsNoiseTexture(mpResourceManager, mpGraphicsContext, mpJobManager);
		if (RC_OK != result)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	void CWeatherSystem::InjectBindings(IWorld* pWorld)
	{
	}


	TDE2_STATIC_CONSTEXPR F32 EarthRadius = 600000.0f;


	void CWeatherSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CWeatherSystem::Update");

		CEntity* pEntity = pWorld->FindEntity(pWorld->FindEntityWithUniqueComponent<CWeatherComponent>());
		if (!pEntity)
		{
			return;
		}

		CWeatherComponent* pWeatherComponent = pEntity->GetComponent<CWeatherComponent>();
		if (!pWeatherComponent)
		{
			return;
		}

		struct
		{
			TVector4 mAtmosphereParameters; // x - Earth's radius, y - inner atmosphere's radius, z - thickness
			TVector4 mWindParameters; // xy - direction, w - scale factor
		} uniformsData;

		uniformsData.mAtmosphereParameters = TVector4{ EarthRadius, 1500.0f, pWeatherComponent->mAtmosphereThickness, 0.0f };
		uniformsData.mWindParameters = TVector4{ pWeatherComponent->mWindDirection.x, pWeatherComponent->mWindDirection.y, 0.0f, 1.0f };

		const TResourceId volumetricCloudsShaderHandle = mpResourceManager->Load<IMaterial>(CProjectSettings::Get()->mGraphicsSettings.mVolumetricCloudsMainShader);
		TDE2_ASSERT(TResourceId::Invalid != volumetricCloudsShaderHandle);

		if (auto pVolumetricCloudsMainShader = mpResourceManager->GetResource<IShader>(volumetricCloudsShaderHandle))
		{
			pVolumetricCloudsMainShader->SetUserUniformsBuffer(1, reinterpret_cast<const U8*>(&uniformsData), sizeof(uniformsData));
		}
	}


	TDE2_API ISystem* CreateWeatherSystem(const TWeatherSystemInitParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CWeatherSystem, result, params);
	}
}