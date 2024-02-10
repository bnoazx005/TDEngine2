#include "../../include/graphics/CFramePostProcessor.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/IGlobalShaderProperties.h"
#include "../../include/graphics/IPostProcessingProfile.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/graphics/IShader.h"
#include "../../include/graphics/ITexture3D.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/core/IWindowSystem.h"
#include "../../include/graphics/CBaseRenderTarget.h"
#include "../../include/graphics/CBaseTexture2D.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/math/MathUtils.h"


namespace TDEngine2
{
	TDE2_STATIC_CONSTEXPR U32 LuminanceTargetSizes = 1024;


	static TResult<TResourceId> GetOrCreateLuminanceTarget(TPtr<IResourceManager> pResourceManager, const std::string& name, U32 sizes)
	{
		TRenderTargetParameters luminanceTargetParams;
		luminanceTargetParams.mWidth = sizes;
		luminanceTargetParams.mHeight = sizes;
		luminanceTargetParams.mFormat = FT_FLOAT1;
		luminanceTargetParams.mNumOfMipLevels = static_cast<U32>(log2(sizes)) + 1;
		luminanceTargetParams.mNumOfSamples = 1;
		luminanceTargetParams.mSamplingQuality = 0;
		luminanceTargetParams.mType = TRenderTargetParameters::E_TARGET_TYPE::TEXTURE2D;

		const TResourceId targetHandle = pResourceManager->Create<IRenderTarget>(name, luminanceTargetParams);
		if (TResourceId::Invalid == targetHandle)
		{
			TDE2_ASSERT(false);
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		if (auto pTarget = pResourceManager->GetResource<IRenderTarget>(targetHandle))
		{
			pTarget->SetUWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);
			pTarget->SetVWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);

			pTarget->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);
		}

		return Wrench::TOkValue<TResourceId>(targetHandle);
	}


	static TResourceId GetRenderTarget(TPtr<IResourceManager> pResourceManager, U32 width, U32 height, bool isHDRSupport, E_FRAME_RENDER_PARAMS_FLAGS flags)
	{
		static U32 counter = 0;

		std::string renderTargetName;

		if (E_FRAME_RENDER_PARAMS_FLAGS::RENDER_MAIN == (E_FRAME_RENDER_PARAMS_FLAGS::RENDER_MAIN & flags))
		{
			renderTargetName = "MainRenderTarget";
		}
		else if (E_FRAME_RENDER_PARAMS_FLAGS::RENDER_UI == (E_FRAME_RENDER_PARAMS_FLAGS::RENDER_UI & flags))
		{
			renderTargetName = "UITarget";
		}
		else
		{
			renderTargetName = "SecondaryRenderTarget" + std::to_string(counter++);
		}

		return pResourceManager->Create<IRenderTarget>(renderTargetName, TTexture2DParameters{ width, height, isHDRSupport ? FT_FLOAT4 : FT_NORM_UBYTE4, 1, 1, 0 });
	}


	CFramePostProcessor::CFramePostProcessor() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CFramePostProcessor::Init(const TFramePostProcessorParameters& desc)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!desc.mpRenderer || !desc.mpGraphicsObjectManager || !desc.mpWindowSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = desc.mpRenderer->GetResourceManager();
		mpGlobalShaderProperties = desc.mpRenderer->GetGlobalShaderProperties().Get();

		mpPreUIRenderQueue     = desc.mpRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_SPRITES);
		mpOverlayRenderQueue   = desc.mpRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_OVERLAY);
		mpWindowSystem         = desc.mpWindowSystem;
		mpGraphicsContext      = desc.mpGraphicsObjectManager->GetGraphicsContext();
		mRenderTargetHandle    = TResourceId::Invalid;
		mUITargetHandle        = TResourceId::Invalid;

		auto luminanceTargetResult = GetOrCreateLuminanceTarget(mpResourceManager, "LuminanceTarget", LuminanceTargetSizes);
		if (luminanceTargetResult.HasError())
		{
			return luminanceTargetResult.GetError();
		}

		mLuminanceTargetHandle = luminanceTargetResult.Get();

		std::array<TResult<TResourceId>, 2> frameLuminanceBuffers
		{
			GetOrCreateLuminanceTarget(mpResourceManager, "LuminancePrevFrame", 1),
			GetOrCreateLuminanceTarget(mpResourceManager, "LuminanceCurrFrame", 1)
		};

		std::transform(frameLuminanceBuffers.begin(), frameLuminanceBuffers.end(), mFramesLuminanceHistoryTargets.begin(), [](const TResult<TResourceId>& result)
		{
			TDE2_ASSERT(result.IsOk());
			return result.Get();
		});

		// Used materials 
		// \todo Refactor this later
		mDefaultScreenSpaceMaterialHandle = mpResourceManager->Create<IMaterial>("DefaultScreenSpaceEffect.material", TMaterialParameters{ "Shaders/PostEffects/BlitTexture.shader", false, TDepthStencilStateDesc { false, false } });
		mBloomFilterMaterialHandle = mpResourceManager->Create<IMaterial>("BloomScreenSpaceEffect.material", TMaterialParameters{ "Shaders/PostEffects/Bloom.shader", false, TDepthStencilStateDesc { false, false } });
		mBloomFinalPassMaterialHandle = mpResourceManager->Create<IMaterial>("BloomFinalPassSpaceEffect.material", TMaterialParameters{ "Shaders/PostEffects/BloomFinal.shader", false, TDepthStencilStateDesc { false, false } });
		mGaussianBlurMaterialHandle = mpResourceManager->Create<IMaterial>("GaussianBlurSpaceEffect.material", TMaterialParameters{ "Shaders/PostEffects/GaussianBlur.shader", false, TDepthStencilStateDesc { false, false } });
		mToneMappingPassMaterialHandle = mpResourceManager->Create<IMaterial>("ToneMappingPass.material", TMaterialParameters{ "Shaders/PostEffects/ToneMapping.shader", false, TDepthStencilStateDesc { false, false } });
		mGenerateLuminanceMaterialHandle = mpResourceManager->Create<IMaterial>("GenerateLuminance.material", TMaterialParameters{ "Shaders/PostEffects/GenerateLuminance.shader", false, TDepthStencilStateDesc { false, false } });
		mLuminanceAdaptationMaterialHandle = mpResourceManager->Create<IMaterial>("AdaptLuminance.material", TMaterialParameters{ "Shaders/PostEffects/AdaptLuminance.shader", false, TDepthStencilStateDesc { false, false } });
		
		mVolumetricCloudsComputeShaderHandle = mpResourceManager->Load<IShader>("Shaders/Default/VolumetricClouds.cshader");
		mVolumetricCloudsComposeMaterialHandle = mpResourceManager->Create<IMaterial>("VolumetricCloudsCompose.material", TMaterialParameters{ "Shaders/Default/VolumetricCloudsCompose.shader", false, TDepthStencilStateDesc { false, false } });

		if (auto vertexFormatResult = desc.mpGraphicsObjectManager->CreateVertexDeclaration())
		{
			mpVertexFormatDeclaration = vertexFormatResult.Get();
		}

		TDE2_ASSERT(mpVertexFormatDeclaration);
		mpVertexFormatDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION });

		const void* pScreenTriangleVerts = static_cast<const void*>(&desc.mpGraphicsObjectManager->GetScreenTriangleVertices()[0]);

		if (auto vertexBufferResult = desc.mpGraphicsObjectManager->CreateBuffer(
			{					
				E_BUFFER_USAGE_TYPE::STATIC,
				E_BUFFER_TYPE::VERTEX,
				3 * sizeof(TVector4),
				pScreenTriangleVerts
			}))
		{
			mFullScreenTriangleVertexBufferHandle = vertexBufferResult.Get();
		}

		TDE2_ASSERT(TBufferHandleId::Invalid != mFullScreenTriangleVertexBufferHandle);

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CFramePostProcessor::SetProcessingProfile(const IPostProcessingProfile* pProfileResource)
	{
		if (!pProfileResource)
		{
			return RC_INVALID_ARGS;
		}

		mpCurrPostProcessingProfile = pProfileResource;

		return RC_OK;
	}


	static const std::string FrontFrameTextureUniformId = "FrameTexture";
	static const std::string BackFrameTextureUniformId = "FrameTexture1";


	E_RESULT_CODE CFramePostProcessor::Render(const TRenderFrameCallback& onRenderFrameCallback, E_FRAME_RENDER_PARAMS_FLAGS flags)
	{
		if (!onRenderFrameCallback)
		{
			LOG_WARNING("[FramePostProcessor] Render method was got empty \"onRenderFrameCallback\" argument");
			return RC_INVALID_ARGS;
		}

		const bool bindDepthBuffer = E_FRAME_RENDER_PARAMS_FLAGS::BIND_DEPTH_BUFFER == (flags & E_FRAME_RENDER_PARAMS_FLAGS::BIND_DEPTH_BUFFER);
		const bool clearRT = E_FRAME_RENDER_PARAMS_FLAGS::CLEAR_RENDER_TARGET == (flags & E_FRAME_RENDER_PARAMS_FLAGS::CLEAR_RENDER_TARGET);
		const bool isMainTarget = E_FRAME_RENDER_PARAMS_FLAGS::RENDER_MAIN == (flags & E_FRAME_RENDER_PARAMS_FLAGS::RENDER_MAIN);


		TPtr<IRenderTarget> pCurrRenderTarget = mpResourceManager->GetResource<IRenderTarget>(isMainTarget ? mRenderTargetHandle : mUITargetHandle);
		TPtr<IDepthBufferTarget> pMainDepthBuffer = mpResourceManager->GetResource<IDepthBufferTarget>(mMainDepthBufferHandle);

		{
			mpGraphicsContext->BindRenderTarget(0, pCurrRenderTarget.Get());
			if (bindDepthBuffer)
			{
				mpGraphicsContext->BindDepthBufferTarget(pMainDepthBuffer.Get());
			}
			
			if (clearRT)
			{
				mpGraphicsContext->ClearRenderTarget(pCurrRenderTarget.Get(), TColor32F{});
			}

			onRenderFrameCallback();
			
			if (bindDepthBuffer)
			{
				mpGraphicsContext->BindDepthBufferTarget(nullptr);
			}

			mpGraphicsContext->BindRenderTarget(0, nullptr);
		}

		return RC_OK;
	}

	E_RESULT_CODE CFramePostProcessor::PreRender()
	{
		const U32 width = mpWindowSystem->GetWidth();
		const U32 height = mpWindowSystem->GetHeight();

		const auto& toneMappingParameters = mpCurrPostProcessingProfile->GetToneMappingParameters();
		const auto& colorGradingParameters = mpCurrPostProcessingProfile->GetColorGradingParameters();

		const bool isHDREnabled = toneMappingParameters.mIsEnabled;

		if (auto pToneMappingMaterial = mpResourceManager->GetResource<IMaterial>(mToneMappingPassMaterialHandle))
		{
			pToneMappingMaterial->SetVariableForInstance<TVector4>(DefaultMaterialInstanceId, "toneMappingParams", TVector4(isHDREnabled ? 1.0f : 0.0f, toneMappingParameters.mExposure, toneMappingParameters.mKeyValue, 0.0f));
			pToneMappingMaterial->SetVariableForInstance<TVector4>(DefaultMaterialInstanceId, "colorGradingParams", TVector4(colorGradingParameters.mIsEnabled ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f));
		}

		_prepareRenderTargetsChain(width, height, isHDREnabled);

		return RC_OK;
	}

	E_RESULT_CODE CFramePostProcessor::RunPostProcess()
	{
		TPtr<IRenderTarget> pCurrRenderTarget  = mpResourceManager->GetResource<IRenderTarget>(mRenderTargetHandle);
		TPtr<IRenderTarget> pBloomRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mBloomRenderTargetHandle);
		TPtr<IRenderTarget> pTempRenderTarget  = mpResourceManager->GetResource<IRenderTarget>(mTemporaryRenderTargetHandle);

		{ // Luminance calculations
#if TDE2_EDITORS_ENABLED
			auto pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(mpGraphicsContext);
			TDE_RENDER_SECTION(pGraphicsContext, "EyeAdaptation");
#endif

			TPtr<IRenderTarget> pLuminanceTarget = mpResourceManager->GetResource<IRenderTarget>(mLuminanceTargetHandle);

			_renderTargetToTarget(pCurrRenderTarget, nullptr, pLuminanceTarget, mGenerateLuminanceMaterialHandle); // Extract luminance
			pLuminanceTarget->GenerateMipMaps();

			// Eye adaptation 
			TPtr<IRenderTarget> pLuminanceAdaptPrevTarget = mpResourceManager->GetResource<IRenderTarget>(mFramesLuminanceHistoryTargets[(mCurrActiveLuminanceFrameTargetIndex + 1) & 0x1]);
			TPtr<IRenderTarget> pLuminanceAdaptCurrTarget = mpResourceManager->GetResource<IRenderTarget>(mFramesLuminanceHistoryTargets[mCurrActiveLuminanceFrameTargetIndex]);

			if (auto pMaterial = mpResourceManager->GetResource<IMaterial>(mLuminanceAdaptationMaterialHandle))
			{
				const auto& toneMappingParameters = mpCurrPostProcessingProfile->GetToneMappingParameters();

				pMaterial->SetVariableForInstance<F32>(DefaultMaterialInstanceId, "mAdaptationRate", toneMappingParameters.mEyeAdaptionCoeff);
			}

			_renderTargetToTarget(pLuminanceTarget, pLuminanceAdaptPrevTarget.Get(), pLuminanceAdaptCurrTarget, mLuminanceAdaptationMaterialHandle);
		}

		_processBloomPass(pCurrRenderTarget, pTempRenderTarget, pBloomRenderTarget);

		mpPreUIRenderQueue->Clear(); // commands above are executed immediately, so we don't need to store them anymore

		return RC_OK;
	}
	
	E_RESULT_CODE CFramePostProcessor::PostRender()
	{
		_submitFullScreenTriangle(mpOverlayRenderQueue, mToneMappingPassMaterialHandle);

		mCurrActiveLuminanceFrameTargetIndex = (mCurrActiveLuminanceFrameTargetIndex + 1) & 0x1;

		return RC_OK;
	}


	E_RESULT_CODE CFramePostProcessor::RunVolumetricCloudsPass()
	{
		TDE2_PROFILER_SCOPE("CFramePostProcessor::RunVolumetricCloudsPass");

		auto pVolumetricCloudsScreenBufferTexture = mpResourceManager->GetResource<ITexture2D>(mVolumetricCloudsScreenBufferHandle);
		if (!pVolumetricCloudsScreenBufferTexture)
		{
			return RC_FAIL;
		}

		auto pDepthBufferResource = mpResourceManager->GetResource<IDepthBufferTarget>(mMainDepthBufferHandle);

		TDE2_STATIC_CONSTEXPR F32 EarthRadius = 6378000.0f;
		TDE2_STATIC_CONSTEXPR F32 AtmosphereThickness = 10000.0f; // \todo Replace with value from component WeatherComponent later

		struct
		{
			TVector4 mAtmosphereParameters;
			TVector2 mInvTextureSizes;
			I32      mStepsCount;
		} uniformsData;

		uniformsData.mAtmosphereParameters = TVector4{ EarthRadius, EarthRadius + 10000.0f, EarthRadius + 10000.0f + AtmosphereThickness, AtmosphereThickness };
		uniformsData.mInvTextureSizes = TVector2
		{
			1 / static_cast<F32>(pVolumetricCloudsScreenBufferTexture->GetWidth()), 1 / static_cast<F32>(pVolumetricCloudsScreenBufferTexture->GetHeight())
		};
		uniformsData.mStepsCount = 64;

		auto pVolumetricCloudsRenderPassShader = mpResourceManager->GetResource<IShader>(mVolumetricCloudsComputeShaderHandle);
		pVolumetricCloudsRenderPassShader->SetTextureResource("OutputTexture", pVolumetricCloudsScreenBufferTexture.Get());
		pVolumetricCloudsRenderPassShader->SetTextureResource("DepthTexture", pDepthBufferResource.Get());

		/// \todo Replace hardcoded values
		{
			auto pCloudsHighNoiseTex = mpResourceManager->GetResource<ITexture3D>(mpResourceManager->Load<ITexture3D>("CloudsHighFreqNoise"));
			auto pCloudsLowNoiseTex = mpResourceManager->GetResource<ITexture3D>(mpResourceManager->Load<ITexture3D>("CloudsLowFreqNoise"));

			pVolumetricCloudsRenderPassShader->SetTextureResource("LowFreqCloudsNoiseTex", pCloudsLowNoiseTex.Get());
			pVolumetricCloudsRenderPassShader->SetTextureResource("HiFreqCloudsNoiseTex", pCloudsHighNoiseTex.Get());
		}

		pVolumetricCloudsRenderPassShader->SetUserUniformsBuffer(0, reinterpret_cast<const U8*>(&uniformsData), sizeof(uniformsData));
		pVolumetricCloudsRenderPassShader->Bind();

		mpGraphicsContext->DispatchCompute(pVolumetricCloudsScreenBufferTexture->GetWidth() / 16, pVolumetricCloudsScreenBufferTexture->GetHeight() / 16, 1);

		pVolumetricCloudsRenderPassShader->Unbind();

		// Compose pass
		TPtr<IRenderTarget> pMainRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mRenderTargetHandle);
		TPtr<IRenderTarget> pTempRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mTemporaryRenderTargetHandle);
		TPtr<ITexture2D> pCloudsRenderTarget = mpResourceManager->GetResource<ITexture2D>(mVolumetricCloudsScreenBufferHandle);

		_renderTargetToTarget(pMainRenderTarget, pCloudsRenderTarget.Get(), pTempRenderTarget, mVolumetricCloudsComposeMaterialHandle); // Compose
		_renderTargetToTarget(pTempRenderTarget, nullptr, pMainRenderTarget, mDefaultScreenSpaceMaterialHandle); // Blit Temp -> Main RT

		return RC_OK;
	}


	static TResult<TResourceId> GetOrCreateVolumetricCloudsBuffer(TPtr<IResourceManager> pResourceManager, U32 width, U32 height)
	{
		TTexture2DParameters textureParams
		{
			width,
			height,
			FT_FLOAT4, 1, 1, 0
		};
		textureParams.mIsWriteable = true;

		const TResourceId textureHandle = pResourceManager->Create<ITexture2D>("VolumetricCloudsScreenBuffer", textureParams);
		if (auto pScreenBufferTexture = pResourceManager->GetResource<ITexture2D>(textureHandle))
		{
			pScreenBufferTexture->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);

			if (pScreenBufferTexture->GetWidth() != width || pScreenBufferTexture->GetHeight() != height)
			{
				LOG_MESSAGE(Wrench::StringUtils::Format("[CFramePostProcessor] The sizes has of volumetric clouds buffer been changed", width, height));
				pScreenBufferTexture->Resize(width, height);
			}
		}

		if (TResourceId::Invalid == textureHandle)
		{
			TDE2_ASSERT(false);
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<TResourceId>(textureHandle);
	}


	static TResult<TResourceId> GetOrCreateDepthBuffer(TPtr<IResourceManager> pResourceManager, U32 width, U32 height)
	{
		TRenderTargetParameters depthBufferParams;
		depthBufferParams.mWidth = width;
		depthBufferParams.mHeight = height;
		depthBufferParams.mFormat = FT_D32;
		depthBufferParams.mNumOfMipLevels = 1;
		depthBufferParams.mNumOfSamples = 1;
		depthBufferParams.mSamplingQuality = 0;
		depthBufferParams.mType = TRenderTargetParameters::E_TARGET_TYPE::TEXTURE2D;

		const TResourceId depthBufferHandle = pResourceManager->Create<IDepthBufferTarget>("MainDepthBuffer", depthBufferParams);
		if (TResourceId::Invalid == depthBufferHandle)
		{
			TDE2_ASSERT(false);
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		if (auto pDepthBufferTexture = pResourceManager->GetResource<IDepthBufferTarget>(depthBufferHandle))
		{
			pDepthBufferTexture->SetUWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);
			pDepthBufferTexture->SetVWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);
			pDepthBufferTexture->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);

			if (pDepthBufferTexture->GetWidth() != width || pDepthBufferTexture->GetHeight() != height)
			{
				LOG_MESSAGE("[CFramePostProcessor] The sizes has been changed");
				pDepthBufferTexture->Resize(width, height);
			}
		}

		return Wrench::TOkValue<TResourceId>(depthBufferHandle);
	}


	void CFramePostProcessor::_prepareRenderTargetsChain(U32 width, U32 height, bool isHDRSupport)
	{
		auto createVolumetricCloudsBufferResult = GetOrCreateVolumetricCloudsBuffer(mpResourceManager, width / 4, height / 4);
		if (createVolumetricCloudsBufferResult.IsOk())
		{
			mVolumetricCloudsScreenBufferHandle = createVolumetricCloudsBufferResult.Get();
		}

		if (TResourceId::Invalid != mRenderTargetHandle)
		{
			if (auto pCurrRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mRenderTargetHandle))
			{
				if (pCurrRenderTarget && (pCurrRenderTarget->GetWidth() != width || pCurrRenderTarget->GetHeight() != height))
				{
					_resizeRenderTargetsChain(width, height);
				}
			}

			return;
		}

		mRenderTargetHandle = GetRenderTarget(mpResourceManager, width, height, isHDRSupport, E_FRAME_RENDER_PARAMS_FLAGS::RENDER_MAIN);
		mTemporaryRenderTargetHandle = GetRenderTarget(mpResourceManager, width, height, isHDRSupport, E_FRAME_RENDER_PARAMS_FLAGS::NONE);
		mUITargetHandle = GetRenderTarget(mpResourceManager, width, height, false, E_FRAME_RENDER_PARAMS_FLAGS::RENDER_UI);

		auto mainDepthBufferRetrieveResult = GetOrCreateDepthBuffer(mpResourceManager, width, height);
		if (mainDepthBufferRetrieveResult.IsOk())
		{
			mMainDepthBufferHandle = mainDepthBufferRetrieveResult.Get();
		}

		auto pCurrRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mRenderTargetHandle);
		auto pUIRenderTarget   = mpResourceManager->GetResource<IRenderTarget>(mUITargetHandle);
		auto pTempRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mTemporaryRenderTargetHandle);
		auto pLuminanceTarget  = mpResourceManager->GetResource<IRenderTarget>(mFramesLuminanceHistoryTargets[mCurrActiveLuminanceFrameTargetIndex]);

		pCurrRenderTarget->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);
		pTempRenderTarget->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);

		const bool isPostProcessingEnabled = mpCurrPostProcessingProfile->IsPostProcessingEnabled();

		if (auto pToneMappingMaterial = mpResourceManager->GetResource<IMaterial>(mToneMappingPassMaterialHandle))
		{
			pToneMappingMaterial->SetTextureResource(FrontFrameTextureUniformId, pCurrRenderTarget.Get());

			/// \note If the color grading's pass was enabled assign its look up texture (assume that it has linear sampling)
			{
				const auto& colorGradingParameters = mpCurrPostProcessingProfile->GetColorGradingParameters();

				if (isPostProcessingEnabled/* && colorGradingParameters.mIsEnabled*/)
				{
					if (auto pColorLUT = mpResourceManager->GetResource<ITexture>(mpResourceManager->Load<ITexture2D>(colorGradingParameters.mLookUpTextureId)))
					{
						pToneMappingMaterial->SetTextureResource("ColorGradingLUT", pColorLUT.Get());
					}
				}

				pToneMappingMaterial->SetTextureResource("LuminanceBuffer", pLuminanceTarget.Get());
				pToneMappingMaterial->SetTextureResource("UIBuffer", pUIRenderTarget.Get());
			}
		}

		if (isPostProcessingEnabled && mpCurrPostProcessingProfile->GetBloomParameters().mIsEnabled)
		{
			const U16 downsampleCoeff = 1 << (TPostProcessingProfileParameters::TBloomParameters::mMaxQuality - mpCurrPostProcessingProfile->GetBloomParameters().mQuality + 1);

			mBloomRenderTargetHandle = GetRenderTarget(mpResourceManager, width / downsampleCoeff, height / downsampleCoeff, isHDRSupport, E_FRAME_RENDER_PARAMS_FLAGS::NONE);

			if (auto pBloomRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mBloomRenderTargetHandle))
			{
				pBloomRenderTarget->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);
			}
		}		
	}

	void CFramePostProcessor::_resizeRenderTargetsChain(U32 width, U32 height)
	{
		const std::array<TResourceId, 4> renderTargetHandles
		{
			mRenderTargetHandle,
			mUITargetHandle,
			mBloomRenderTargetHandle,
			mTemporaryRenderTargetHandle
		};

		for (const TResourceId& currTargetHandle : renderTargetHandles)
		{
			if (auto pCurrRenderTarget = mpResourceManager->GetResource<IRenderTarget>(currTargetHandle))
			{
				pCurrRenderTarget->Resize(width, height);
			}
		}
	}

	void CFramePostProcessor::_processBloomPass(TPtr<IRenderTarget> pFrontTarget, TPtr<IRenderTarget> pBackTarget, TPtr<IRenderTarget> pBloomTarget)
	{
		const auto& bloomParameters = mpCurrPostProcessingProfile->GetBloomParameters();
		const auto& toneMappingParameters = mpCurrPostProcessingProfile->GetToneMappingParameters();

		if (!mpCurrPostProcessingProfile->IsPostProcessingEnabled() || !bloomParameters.mIsEnabled)
		{
			return;
		}

#if TDE2_EDITORS_ENABLED
		auto pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(mpGraphicsContext);
		TDE_RENDER_SECTION(pGraphicsContext, "Bloom");
#endif

		if (auto pBloomMaterial = mpResourceManager->GetResource<IMaterial>(mBloomFilterMaterialHandle))
		{
			pBloomMaterial->SetVariableForInstance<F32>(DefaultMaterialInstanceId, "threshold", bloomParameters.mThreshold);
			pBloomMaterial->SetVariableForInstance<F32>(DefaultMaterialInstanceId, "keyValue", toneMappingParameters.mKeyValue);
		}

		const TVector4 blurParams{ bloomParameters.mSmoothness, 0.0, 1.0f / static_cast<F32>(pFrontTarget->GetWidth()), 1.0f / static_cast<F32>(pFrontTarget->GetHeight()) };
		const TVector4 vertBlurRotation{ 0.0f, CMathConstants::Pi * 0.5f, 0.0f, 0.0f };

		if (auto pBlurMaterial = mpResourceManager->GetResource<IMaterial>(mGaussianBlurMaterialHandle))
		{
			pBlurMaterial->SetVariableForInstance<TVector4>(DefaultMaterialInstanceId, "blurParams", blurParams);
			pBlurMaterial->SetVariableForInstance<U32>(DefaultMaterialInstanceId, "samplesCount", bloomParameters.mSamplesCount);
		}

		auto pLuminanceTarget = mpResourceManager->GetResource<IRenderTarget>(mFramesLuminanceHistoryTargets[mCurrActiveLuminanceFrameTargetIndex]);

		_renderTargetToTarget(pFrontTarget, pLuminanceTarget.Get(), pBloomTarget, mBloomFilterMaterialHandle); // Bloom pass
		_renderTargetToTarget(pBloomTarget, nullptr, pBackTarget, mGaussianBlurMaterialHandle); // Horizontal Blur pass

		if (auto pBlurMaterial = mpResourceManager->GetResource<IMaterial>(mGaussianBlurMaterialHandle))
		{
			pBlurMaterial->SetVariableForInstance<TVector4>(DefaultMaterialInstanceId, "blurParams", blurParams + vertBlurRotation);
		}

		_renderTargetToTarget(pBackTarget, nullptr, pBloomTarget, mGaussianBlurMaterialHandle); // Vertical Blur pass
		_renderTargetToTarget(pFrontTarget, pBloomTarget.Get(), pBackTarget, mBloomFinalPassMaterialHandle); // Compose
		_renderTargetToTarget(pBackTarget, nullptr, pFrontTarget, mDefaultScreenSpaceMaterialHandle); // Blit Temp -> Main render target
	}

	void CFramePostProcessor::_submitFullScreenTriangle(CRenderQueue* pRenderQueue, TResourceId materialHandle, bool drawImmediately)
	{
		// \note The fullscreen triangle is drawn without vertex buffer, all the geometry is generated inside the vertex shader
		TDrawCommandPtr pDrawCommand      = pRenderQueue->SubmitDrawCommand<TDrawCommand>(static_cast<U32>(E_GEOMETRY_SUBGROUP_TAGS::IMAGE_EFFECTS));
		pDrawCommand->mNumOfVertices      = 3;
		pDrawCommand->mPrimitiveType      = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
		pDrawCommand->mMaterialHandle     = materialHandle;
		pDrawCommand->mVertexBufferHandle = mFullScreenTriangleVertexBufferHandle;
		pDrawCommand->mpVertexDeclaration = mpVertexFormatDeclaration;

		if (drawImmediately)
		{
			pDrawCommand->Submit(mpGraphicsContext, mpResourceManager.Get(), mpGlobalShaderProperties);
		}
	}
	 
	void CFramePostProcessor::_renderTargetToTarget(TPtr<IRenderTarget> pSource, ITexture* pExtraSource, TPtr<IRenderTarget> pDest, TResourceId materialHandle)
	{		
		mpGraphicsContext->SetDepthBufferEnabled(false);
		mpGraphicsContext->BindRenderTarget(0, pDest.Get());
		mpGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(pDest->GetWidth()), static_cast<F32>(pDest->GetHeight()), 0.0f, 1.0f);

		if (auto pMaterial = mpResourceManager->GetResource<IMaterial>(materialHandle))
		{
			pMaterial->SetTextureResource(FrontFrameTextureUniformId, pSource.Get());

			if (pExtraSource)
			{
				pMaterial->SetTextureResource(BackFrameTextureUniformId, pExtraSource);
			}
		}

		_submitFullScreenTriangle(mpPreUIRenderQueue, materialHandle, true);

		mpGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(mpWindowSystem->GetWidth()), static_cast<F32>(mpWindowSystem->GetHeight()), 0.0f, 1.0f);
		mpGraphicsContext->SetDepthBufferEnabled(true);
		mpGraphicsContext->BindRenderTarget(0, nullptr);
	}


	TDE2_API IFramePostProcessor* CreateFramePostProcessor(const TFramePostProcessorParameters& desc, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IFramePostProcessor, CFramePostProcessor, result, desc);
	}
}