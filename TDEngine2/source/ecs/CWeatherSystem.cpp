/*
	Clouds rendering based on implementation described at
	https://www.guerrilla-games.com/read/the-real-time-volumetric-cloudscapes-of-horizon-zero-dawn
*/

#include "../../include/ecs/CWeatherSystem.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IJobManager.h"
#include "../../include/graphics/CBaseTexture3D.h"
#include "../../include/graphics/IMaterial.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/math/MathUtils.h"
#include "../../include/utils/CFileLogger.h"
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
		pJobManager->SubmitJob(nullptr, [pJobManager, pResourceManager, lowFreqCloudsNoiseTexHandle](auto)
		{
			LOG_MESSAGE("[GenerateCloudsNoiseTexture] Start generate low frequency clouds noise 3D texture...");

			std::vector<U8> lowFreqNoisePixelsData(LowFreqCloudsNoiseTextureSize * LowFreqCloudsNoiseTextureSize * LowFreqCloudsNoiseTextureSize * 4);

			CPerlinNoise perlinNoiseGenerator(Wrench::Random<U32, F32>(0).Get(0, std::numeric_limits<U32>::max()));
			CWorleyNoise worleyNoiseGenerator;

			auto remap = [](F32 perlin, F32 oldMin, F32 oldMax, F32 newMin, F32 newMax)
			{
				return (((perlin - oldMin) / (oldMax - oldMin)) * (newMax - newMin)) + newMin;
			};

			TJobCounter counter;

			pJobManager->SubmitMultipleJobs(&counter, LowFreqCloudsNoiseTextureSize, 16, [&](const TJobArgs& jobInfo)
			{
				for (USIZE y = 0; y < LowFreqCloudsNoiseTextureSize; y++)
				{
					for (USIZE x = 0; x < LowFreqCloudsNoiseTextureSize; x++)
					{
						const USIZE startIndex = 4 * (x + LowFreqCloudsNoiseTextureSize * y + LowFreqCloudsNoiseTextureSize * LowFreqCloudsNoiseTextureSize * jobInfo.mJobIndex);

						const TVector3 p0(static_cast<F32>(x), static_cast<F32>(y), static_cast<F32>(jobInfo.mJobIndex));
						const TVector3 p1 = 2.0f * Scale(p0, TVector3(InvLowFreqCloudsNoiseTextureSize)) - TVector3(1.0f);

						const F32 perlinNoise = CMathUtils::Abs(2.0f * perlinNoiseGenerator.Compute3D(p0, 3) - 1.0f);
						const F32 worleyNoise = worleyNoiseGenerator.Compute3D(p1, 3, 2.0f);

						lowFreqNoisePixelsData[startIndex] = static_cast<U8>(255.0f * remap(perlinNoise, 0.0f, 1.0f, worleyNoise, 1.0f));
						lowFreqNoisePixelsData[startIndex + 1] = static_cast<U8>(255.0f * worleyNoiseGenerator.Compute3D(p1, 3, 4.0f));
						lowFreqNoisePixelsData[startIndex + 2] = static_cast<U8>(255.0f * worleyNoiseGenerator.Compute3D(p1, 3, 8.0f));
						lowFreqNoisePixelsData[startIndex + 3] = static_cast<U8>(255.0f * worleyNoiseGenerator.Compute3D(p1, 3, 8.0f));
					}
				}
			});

			pJobManager->WaitForJobCounter(counter);

			pJobManager->ExecuteInMainThread([=, pixelsData = std::move(lowFreqNoisePixelsData)]
			{
				LOG_MESSAGE("[GenerateCloudsNoiseTexture] Update GPU texture");

				if (auto pLowFreqNoiseTexture = pResourceManager->GetResource<CBaseTexture3D>(lowFreqCloudsNoiseTexHandle))
				{
					pLowFreqNoiseTexture->WriteData({ 0, 0, static_cast<I32>(LowFreqCloudsNoiseTextureSize), static_cast<I32>(LowFreqCloudsNoiseTextureSize) },
						0, LowFreqCloudsNoiseTextureSize, pixelsData.data());
				}
			});

			LOG_MESSAGE("[GenerateCloudsNoiseTexture] Generation finished");
		}, { E_JOB_PRIORITY_TYPE::NORMAL, false, "GenerateLowFreqCloudsNoiseJob" });

		// \note Generate high frequency clouds noise
		pJobManager->SubmitJob(nullptr, [pJobManager, pResourceManager, highFreqCloudsNoiseTexHandle](auto)
		{
			LOG_MESSAGE("[GenerateCloudsNoiseTexture] Start generate high frequency clouds noise 3D texture...");

			std::vector<U8> highFreqNoisePixelsData(HighFreqCloudsNoiseTextureSize * HighFreqCloudsNoiseTextureSize * HighFreqCloudsNoiseTextureSize * 4); // RGBA, alpha is always 255
			memset(highFreqNoisePixelsData.data(), 0xFF, highFreqNoisePixelsData.size());

			CWorleyNoise worleyNoiseGenerator;

			for (USIZE z = 0; z < HighFreqCloudsNoiseTextureSize; z++)
			{
				for (USIZE y = 0; y < HighFreqCloudsNoiseTextureSize; y++)
				{
					for (USIZE x = 0; x < HighFreqCloudsNoiseTextureSize; x++)
					{
						const USIZE startIndex = 4 * (x + HighFreqCloudsNoiseTextureSize * y + HighFreqCloudsNoiseTextureSize * HighFreqCloudsNoiseTextureSize * z);
						
						const TVector3 point(
							2.0f * static_cast<F32>(x) * InvHighFreqCloudsNoiseTextureSize - 1.0f, 
							2.0f * static_cast<F32>(y) * InvHighFreqCloudsNoiseTextureSize - 1.0f, 
							2.0f * static_cast<F32>(z) * InvHighFreqCloudsNoiseTextureSize - 1.0f);

						highFreqNoisePixelsData[startIndex] = static_cast<U8>(255.0f * worleyNoiseGenerator.Compute3D(point, 3, 2.0f));
						highFreqNoisePixelsData[startIndex + 1] = static_cast<U8>(255.0f * worleyNoiseGenerator.Compute3D(point, 3, 4.0f));
						highFreqNoisePixelsData[startIndex + 2] = static_cast<U8>(255.0f * worleyNoiseGenerator.Compute3D(point, 3, 8.0f));
					}
				}
			}

			pJobManager->ExecuteInMainThread([=, pixelsData = std::move(highFreqNoisePixelsData)]
			{
				LOG_MESSAGE("[GenerateCloudsNoiseTexture] Update GPU texture");

				if (auto pHighFreqNoiseTexture = pResourceManager->GetResource<CBaseTexture3D>(highFreqCloudsNoiseTexHandle))
				{
					pHighFreqNoiseTexture->WriteData({ 0, 0, static_cast<I32>(HighFreqCloudsNoiseTextureSize), static_cast<I32>(HighFreqCloudsNoiseTextureSize) },
						0, HighFreqCloudsNoiseTextureSize, pixelsData.data());
				}
			});

			LOG_MESSAGE("[GenerateCloudsNoiseTexture] Generation finished");
		}, { E_JOB_PRIORITY_TYPE::NORMAL, false, "GenerateHighFreqCloudsNoiseJob" });
	}


	static E_RESULT_CODE CreateCloudsNoiseTexture(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IJobManager* pJobManager)
	{
		const TTexture3DParameters lowFreqCloudsNoiseTextureParams 
		{ 
			LowFreqCloudsNoiseTextureSize, 
			LowFreqCloudsNoiseTextureSize, 
			LowFreqCloudsNoiseTextureSize, 
			FT_NORM_UBYTE4, 1, 1, 0 
		};

		const TTexture3DParameters highFreqCloudsNoiseTextureParams
		{ 
			HighFreqCloudsNoiseTextureSize,
			HighFreqCloudsNoiseTextureSize, 
			HighFreqCloudsNoiseTextureSize, 
			FT_NORM_UBYTE4, 1, 1, 0
		};

		const TResourceId lowFreqCloudsTextureHandle = pResourceManager->Create<ITexture3D>(LowFreqCloudsNoise3DTextureId, lowFreqCloudsNoiseTextureParams);
		const TResourceId highFreqCloudsTextureHandle = pResourceManager->Create<ITexture3D>(HighFreqCloudsNoise3DTextureId, highFreqCloudsNoiseTextureParams);

		auto pMaterial = pResourceManager->GetResource<IMaterial>(pResourceManager->Load<IMaterial>("DefaultResources/Materials/DefaultSkydome.material"));
		pMaterial->SetTextureResource("Test3D", pResourceManager->GetResource<ITexture>(lowFreqCloudsTextureHandle).Get());

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

	void CWeatherSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CWeatherSystem::Update");
	}


	TDE2_API ISystem* CreateWeatherSystem(const TWeatherSystemInitParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CWeatherSystem, result, params);
	}
}