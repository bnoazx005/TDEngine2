#include "../../include/graphics/CFramePostProcessor.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/IGlobalShaderProperties.h"
#include "../../include/graphics/IPostProcessingProfile.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/core/IWindowSystem.h"
#include "../../include/graphics/CBaseRenderTarget.h"
#include "../../include/graphics/CBaseTexture2D.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/math/MathUtils.h"


namespace TDEngine2
{
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

		mpPreUIRenderQueue   = desc.mpRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_SPRITES);
		mpOverlayRenderQueue = desc.mpRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_OVERLAY);
		mpWindowSystem       = desc.mpWindowSystem;
		mpGraphicsContext    = desc.mpGraphicsObjectManager->GetGraphicsContext();
		mRenderTargetHandle  = TResourceId::Invalid;

		// Used materials 
		// \todo Refactor this later
		mDefaultScreenSpaceMaterialHandle = mpResourceManager->Create<IMaterial>("DefaultScreenSpaceEffect.material", TMaterialParameters{ "DefaultScreenSpaceEffect", false, TDepthStencilStateDesc { false, false } });
		mBloomFilterMaterialHandle = mpResourceManager->Create<IMaterial>("BloomScreenSpaceEffect.material", TMaterialParameters{ "Shaders/PostEffects/Bloom.shader", false, TDepthStencilStateDesc { false, false } });
		mBloomFinalPassMaterialHandle = mpResourceManager->Create<IMaterial>("BloomFinalPassSpaceEffect.material", TMaterialParameters{ "Shaders/PostEffects/BloomFinal.shader", false, TDepthStencilStateDesc { false, false } });
		mGaussianBlurMaterialHandle = mpResourceManager->Create<IMaterial>("GaussianBlurSpaceEffect.material", TMaterialParameters{ "Shaders/PostEffects/GaussianBlur.shader", false, TDepthStencilStateDesc { false, false } });
		mToneMappingPassMaterialHandle = mpResourceManager->Create<IMaterial>("ToneMappingPass.material", TMaterialParameters{ "ToneMappingPass", false, TDepthStencilStateDesc { false, false } });

		if (auto vertexFormatResult = desc.mpGraphicsObjectManager->CreateVertexDeclaration())
		{
			mpVertexFormatDeclaration = vertexFormatResult.Get();
		}

		TDE2_ASSERT(mpVertexFormatDeclaration);
		mpVertexFormatDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION });

		const void* pScreenTriangleVerts = static_cast<const void*>(&desc.mpGraphicsObjectManager->GetScreenTriangleVertices()[0]);

		if (auto vertexBufferResult = desc.mpGraphicsObjectManager->CreateVertexBuffer(BUT_STATIC, 3 * sizeof(TVector4), pScreenTriangleVerts))
		{
			mpFullScreenTriangleVertexBuffer = vertexBufferResult.Get();
		}

		TDE2_ASSERT(mpFullScreenTriangleVertexBuffer);

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


	E_RESULT_CODE CFramePostProcessor::Render(const TRenderFrameCallback& onRenderFrameCallback, bool clearRenderTarget)
	{
		if (!onRenderFrameCallback)
		{
			LOG_WARNING("[FramePostProcessor] Render method was got empty \"onRenderFrameCallback\" argument");
			return RC_INVALID_ARGS;
		}

		TPtr<IRenderTarget> pCurrRenderTarget  = mpResourceManager->GetResource<IRenderTarget>(mRenderTargetHandle);

		{
			mpGraphicsContext->BindRenderTarget(0, pCurrRenderTarget.Get());
			
			if (clearRenderTarget)
			{
				mpGraphicsContext->ClearRenderTarget(pCurrRenderTarget.Get(), TColor32F{});
			}

			onRenderFrameCallback();
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
			pToneMappingMaterial->SetVariableForInstance<TVector4>(DefaultMaterialInstanceId, "toneMappingParams", TVector4(isHDREnabled ? 1.0f : 0.0f, toneMappingParameters.mExposure, 0.0f, 0.0f));
			pToneMappingMaterial->SetVariableForInstance<TVector4>(DefaultMaterialInstanceId, "colorGradingParams", TVector4(colorGradingParameters.mIsEnabled ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f));
		}

		_prepareRenderTargetsChain(width, height, isHDREnabled);

		return RC_OK;
	}

	E_RESULT_CODE CFramePostProcessor::RunPostProcess()
	{
		TPtr<IRenderTarget> pCurrRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mRenderTargetHandle);
		TPtr<IRenderTarget> pBloomRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mBloomRenderTargetHandle);
		TPtr<IRenderTarget> pTempRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mTemporaryRenderTargetHandle);

		_processBloomPass(pCurrRenderTarget, pTempRenderTarget, pBloomRenderTarget);

		return RC_OK;
	}
	
	E_RESULT_CODE CFramePostProcessor::PostRender()
	{
		_submitFullScreenTriangle(mpOverlayRenderQueue, mToneMappingPassMaterialHandle);

		return RC_OK;
	}

	void CFramePostProcessor::_prepareRenderTargetsChain(U32 width, U32 height, bool isHDRSupport)
	{
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

		mRenderTargetHandle = _getRenderTarget(width, height, isHDRSupport);
		mTemporaryRenderTargetHandle = _getRenderTarget(width, height, isHDRSupport, false);

		auto pCurrRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mRenderTargetHandle);
		auto pTempRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mTemporaryRenderTargetHandle);

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
			}
		}

		if (isPostProcessingEnabled && mpCurrPostProcessingProfile->GetBloomParameters().mIsEnabled)
		{
			const U16 downsampleCoeff = 1 << (TPostProcessingProfileParameters::TBloomParameters::mMaxQuality - mpCurrPostProcessingProfile->GetBloomParameters().mQuality + 1);

			mBloomRenderTargetHandle = _getRenderTarget(width / downsampleCoeff, height / downsampleCoeff, isHDRSupport, false);

			if (auto pBloomRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mRenderTargetHandle))
			{
				pBloomRenderTarget->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);
			}
		}
	}

	void CFramePostProcessor::_resizeRenderTargetsChain(U32 width, U32 height)
	{
		const std::array<TResourceId, 3> renderTargetHandles
		{
			mRenderTargetHandle,
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

		if (!mpCurrPostProcessingProfile->IsPostProcessingEnabled() || !bloomParameters.mIsEnabled)
		{
			return;
		}

		if (auto pBloomMaterial = mpResourceManager->GetResource<IMaterial>(mBloomFilterMaterialHandle))
		{
			pBloomMaterial->SetVariableForInstance<F32>(DefaultMaterialInstanceId, "threshold", bloomParameters.mThreshold);
		}

		const TVector4 blurParams{ bloomParameters.mSmoothness, 0.0, 1.0f / static_cast<F32>(pFrontTarget->GetWidth()), 1.0f / static_cast<F32>(pFrontTarget->GetHeight()) };
		const TVector4 vertBlurRotation{ 0.0f, CMathConstants::Pi * 0.5f, 0.0f, 0.0f };

		if (auto pBlurMaterial = mpResourceManager->GetResource<IMaterial>(mGaussianBlurMaterialHandle))
		{
			pBlurMaterial->SetVariableForInstance<TVector4>(DefaultMaterialInstanceId, "blurParams", blurParams);
			pBlurMaterial->SetVariableForInstance<U32>(DefaultMaterialInstanceId, "samplesCount", bloomParameters.mSamplesCount);
		}

		_renderTargetToTarget(pFrontTarget, nullptr, pBloomTarget, mBloomFilterMaterialHandle); // Bloom pass
		_renderTargetToTarget(pBloomTarget, nullptr, pBackTarget, mGaussianBlurMaterialHandle); // Horizontal Blur pass

		if (auto pBlurMaterial = mpResourceManager->GetResource<IMaterial>(mGaussianBlurMaterialHandle))
		{
			pBlurMaterial->SetVariableForInstance<TVector4>(DefaultMaterialInstanceId, "blurParams", blurParams + vertBlurRotation);
		}

		_renderTargetToTarget(pBackTarget, nullptr, pBloomTarget, mGaussianBlurMaterialHandle); // Vertical Blur pass
		_renderTargetToTarget(pFrontTarget, pBloomTarget, pBackTarget, mBloomFinalPassMaterialHandle); // Compose
		_renderTargetToTarget(pBackTarget, nullptr, pFrontTarget, mDefaultScreenSpaceMaterialHandle); // Blit Temp -> Main render target

		mpPreUIRenderQueue->Clear(); // commands above are executed immediately, so we don't need to store them anymore
	}

	void CFramePostProcessor::_submitFullScreenTriangle(CRenderQueue* pRenderQueue, TResourceId materialHandle, bool drawImmediately)
	{
		// \note The fullscreen triangle is drawn without vertex buffer, all the geometry is generated inside the vertex shader
		TDrawCommandPtr pDrawCommand      = pRenderQueue->SubmitDrawCommand<TDrawCommand>(static_cast<U32>(E_GEOMETRY_SUBGROUP_TAGS::IMAGE_EFFECTS));
		pDrawCommand->mNumOfVertices      = 3;
		pDrawCommand->mPrimitiveType      = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
		pDrawCommand->mMaterialHandle     = materialHandle;
		pDrawCommand->mpVertexBuffer      = mpFullScreenTriangleVertexBuffer;
		pDrawCommand->mpVertexDeclaration = mpVertexFormatDeclaration;

		if (drawImmediately)
		{
			pDrawCommand->Submit(mpGraphicsContext, mpResourceManager.Get(), mpGlobalShaderProperties);
		}
	}
	 
	void CFramePostProcessor::_renderTargetToTarget(TPtr<IRenderTarget> pSource, TPtr<IRenderTarget> pExtraSource, TPtr<IRenderTarget> pDest, TResourceId materialHandle)
	{
		mpGraphicsContext->SetDepthBufferEnabled(false);
		mpGraphicsContext->BindRenderTarget(0, pDest.Get());
		mpGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(pDest->GetWidth()), static_cast<F32>(pDest->GetHeight()), 0.0f, 1.0f);

		if (auto pMaterial = mpResourceManager->GetResource<IMaterial>(materialHandle))
		{
			pMaterial->SetTextureResource(FrontFrameTextureUniformId, pSource.Get());

			if (pExtraSource)
			{
				pMaterial->SetTextureResource(BackFrameTextureUniformId, pExtraSource.Get());
			}
		}

		_submitFullScreenTriangle(mpPreUIRenderQueue, materialHandle, true);

		mpGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(mpWindowSystem->GetWidth()), static_cast<F32>(mpWindowSystem->GetHeight()), 0.0f, 1.0f);
		mpGraphicsContext->SetDepthBufferEnabled(true);
		mpGraphicsContext->BindRenderTarget(0, nullptr);
	}

	TResourceId CFramePostProcessor::_getRenderTarget(U32 width, U32 height, bool isHDRSupport, bool isMainTarget)
	{
		static const std::string renderTartetIdentifiers[2]
		{
			"MainRenderTarget",
			"SecondaryRenderTarget"
		};

		static U32 counter = 0;

		const std::string renderTargetName = isMainTarget ? renderTartetIdentifiers[0] : renderTartetIdentifiers[1] + std::to_string(counter++);

		return mpResourceManager->Create<IRenderTarget>(renderTargetName, TTexture2DParameters{ width, height, isHDRSupport ? FT_FLOAT4 : FT_NORM_UBYTE4, 1, 1, 0 });
	}


	TDE2_API IFramePostProcessor* CreateFramePostProcessor(const TFramePostProcessorParameters& desc, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IFramePostProcessor, CFramePostProcessor, result, desc);
	}
}