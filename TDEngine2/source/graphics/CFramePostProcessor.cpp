#include "../../include/graphics/CFramePostProcessor.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/IGlobalShaderProperties.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/core/IWindowSystem.h"
#include "../../include/graphics/CBaseRenderTarget.h"
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
		mpGlobalShaderProperties = desc.mpRenderer->GetGlobalShaderProperties();

		mpOverlayRenderQueue              = desc.mpRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_OVERLAY);
		mpWindowSystem                    = desc.mpWindowSystem;
		mpGraphicsContext                 = desc.mpGraphicsObjectManager->GetGraphicsContext();
		mRenderTargetHandle               = TResourceId::Invalid;

		// Used materials 
		// \todo Refactor this later
		mDefaultScreenSpaceMaterialHandle = mpResourceManager->Create<CBaseMaterial>("DefaultScreenSpaceEffect.material", TMaterialParameters{ "DefaultScreenSpaceEffect", false, TDepthStencilStateDesc { false, false } });
		mBloomFilterMaterialHandle = mpResourceManager->Create<CBaseMaterial>("BloomScreenSpaceEffect.material", TMaterialParameters{ "Shaders/PostEffects/DX/Bloom.shader", false, TDepthStencilStateDesc { false, false } });
		mBloomFinalPassMaterialHandle = mpResourceManager->Create<CBaseMaterial>("BloomFinalPassSpaceEffect.material", TMaterialParameters{ "Shaders/PostEffects/DX/BloomFinal.shader", false, TDepthStencilStateDesc { false, false } });
		mGaussianBlurMaterialHandle = mpResourceManager->Create<CBaseMaterial>("GaussianBlurSpaceEffect.material", TMaterialParameters{ "Shaders/PostEffects/DX/GaussianBlur.shader", false, TDepthStencilStateDesc { false, false } });

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

	E_RESULT_CODE CFramePostProcessor::Free()
	{
		if (!mIsInitialized)
		{
			return RC_OK;
		}

		mIsInitialized = false;
		delete this;

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

	E_RESULT_CODE CFramePostProcessor::PreRender()
	{
		TDE2_UNIMPLEMENTED();
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CFramePostProcessor::Render(const TRenderFrameCallback& onRenderFrameCallback)
	{
		if (!onRenderFrameCallback)
		{
			LOG_WARNING("[FramePostProcessor] Render method was got empty \"onRenderFrameCallback\" argument");
			return RC_INVALID_ARGS;
		}

		IRenderTarget* pCurrRenderTarget = nullptr;
		IRenderTarget* pTempRenderTarget = nullptr;
		IRenderTarget* pBloomRenderTarget = nullptr;

		const U32 width = mpWindowSystem->GetWidth();
		const U32 height = mpWindowSystem->GetHeight();

		if (mRenderTargetHandle == TResourceId::Invalid)
		{
			mRenderTargetHandle = _getRenderTarget(width, height, true);
			pCurrRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mRenderTargetHandle);

			if (auto pScreenSpaceMaterial = mpResourceManager->GetResource<IMaterial>(mDefaultScreenSpaceMaterialHandle))
			{
				pScreenSpaceMaterial->SetTextureResource("FrameTexture", pCurrRenderTarget);
			}

			mBloomRenderTargetHandle = _getRenderTarget(width, height, true, false);

			mTemporaryRenderTargetHandle = _getRenderTarget(width, height, true, false);
			pTempRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mTemporaryRenderTargetHandle);
		}

		pCurrRenderTarget = GetValidPtrOrDefault(pCurrRenderTarget, mpResourceManager->GetResource<IRenderTarget>(mRenderTargetHandle));
		pBloomRenderTarget = GetValidPtrOrDefault(pBloomRenderTarget, mpResourceManager->GetResource<IRenderTarget>(mBloomRenderTargetHandle));
		pTempRenderTarget = GetValidPtrOrDefault(pTempRenderTarget, mpResourceManager->GetResource<IRenderTarget>(mTemporaryRenderTargetHandle));

		if (pCurrRenderTarget && (pCurrRenderTarget->GetWidth() != width || pCurrRenderTarget->GetHeight() != height))
		{
			mpResourceManager->ReleaseResource(mRenderTargetHandle);

			mRenderTargetHandle = _getRenderTarget(width, height, true);
			pCurrRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mRenderTargetHandle);
			pCurrRenderTarget->SetFilterType(E_FILTER_TYPE::FT_BILINEAR);

			if (auto pScreenSpaceMaterial = mpResourceManager->GetResource<IMaterial>(mDefaultScreenSpaceMaterialHandle))
			{
				pScreenSpaceMaterial->SetTextureResource("FrameTexture", pCurrRenderTarget);
			}

			mBloomRenderTargetHandle = _getRenderTarget(width, height, true, false);
			pBloomRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mBloomRenderTargetHandle);
			pBloomRenderTarget->SetFilterType(E_FILTER_TYPE::FT_BILINEAR);

			mTemporaryRenderTargetHandle = _getRenderTarget(width, height, true);
			pTempRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mTemporaryRenderTargetHandle);
			pTempRenderTarget->SetFilterType(E_FILTER_TYPE::FT_BILINEAR);
		}

		{
			mpGraphicsContext->BindRenderTarget(0, pCurrRenderTarget);
			onRenderFrameCallback();
			mpGraphicsContext->BindRenderTarget(0, nullptr);
		}

		if (auto pBloomMaterial = mpResourceManager->GetResource<IMaterial>(mBloomFilterMaterialHandle))
		{
			pBloomMaterial->SetVariableForInstance<F32>(DefaultMaterialInstanceId, "threshold", 1.5f);
		}

		const F32 invWidth = 1.0f / static_cast<F32>(width);
		const F32 invHeight = 1.0f / static_cast<F32>(height);

		if (auto pBlurMaterial = mpResourceManager->GetResource<IMaterial>(mGaussianBlurMaterialHandle))
		{
			pBlurMaterial->SetVariableForInstance<TVector4>(DefaultMaterialInstanceId, "blurParams", TVector4(1.2f, 0.0, invWidth, invHeight));
			pBlurMaterial->SetVariableForInstance<U32>(DefaultMaterialInstanceId, "samplesCount", 12);
		}

		_renderTargetToTarget(pCurrRenderTarget, nullptr, pBloomRenderTarget, mBloomFilterMaterialHandle); // Bloom pass

		// \todo Implement this stages
		_renderTargetToTarget(pBloomRenderTarget, nullptr, pTempRenderTarget, mGaussianBlurMaterialHandle); // Horizontal Blur pass

		if (auto pBlurMaterial = mpResourceManager->GetResource<IMaterial>(mGaussianBlurMaterialHandle))
		{
			pBlurMaterial->SetVariableForInstance<TVector4>(DefaultMaterialInstanceId, "blurParams", TVector4(1.2f, CMathConstants::Pi * 0.5f, invWidth, invHeight));
		}

		_renderTargetToTarget(pTempRenderTarget, nullptr, pBloomRenderTarget, mGaussianBlurMaterialHandle); // Vertical Blur pass
		_renderTargetToTarget(pCurrRenderTarget, pBloomRenderTarget, pTempRenderTarget, mBloomFinalPassMaterialHandle); // Compose
		_renderTargetToTarget(pTempRenderTarget, nullptr, pCurrRenderTarget, mDefaultScreenSpaceMaterialHandle); // Blit Temp -> Main render target

		mpOverlayRenderQueue->Clear(); // commands above are executed immediately, so we don't need to store them anymore

		_submitFullScreenTriangle(mpOverlayRenderQueue, mDefaultScreenSpaceMaterialHandle);

		return RC_OK;
	}

	E_RESULT_CODE CFramePostProcessor::PostRender()
	{
		TDE2_UNIMPLEMENTED();
		return RC_NOT_IMPLEMENTED_YET;
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
			pDrawCommand->Submit(mpGraphicsContext, mpResourceManager, mpGlobalShaderProperties);
		}
	}

	void CFramePostProcessor::_renderTargetToTarget(IRenderTarget* pSource, IRenderTarget* pExtraSource, IRenderTarget* pDest, TResourceId materialHandle)
	{
		mpGraphicsContext->BindRenderTarget(0, pDest);
		mpGraphicsContext->SetDepthBufferEnabled(false);
		mpGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(pDest->GetWidth()), static_cast<F32>(pDest->GetHeight()), 0.0f, 1.0f);

		if (auto pMaterial = mpResourceManager->GetResource<IMaterial>(materialHandle))
		{
			pMaterial->SetTextureResource("FrameTexture", pSource);

			if (pExtraSource)
			{
				pMaterial->SetTextureResource("FrameTexture1", pExtraSource);
			}
		}

		_submitFullScreenTriangle(mpOverlayRenderQueue, materialHandle, true);

		mpGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(mpWindowSystem->GetWidth()), static_cast<F32>(mpWindowSystem->GetHeight()), 0.0f, 1.0f);
		mpGraphicsContext->SetDepthBufferEnabled(true);
		mpGraphicsContext->BindRenderTarget(0, nullptr);
	}

	TResourceId CFramePostProcessor::_getRenderTarget(U32 width, U32 height, bool isHDRSupport, bool isMainTarget)
	{
		static U32 counter = 0;

		const std::string renderTargetName = isMainTarget ? "MainRenderTarget" : "SecondaryRenderTarget" + std::to_string(counter++);

		return mpResourceManager->Create<CBaseRenderTarget>(renderTargetName, TTexture2DParameters{ width, height, isHDRSupport ? FT_FLOAT4 : FT_NORM_UBYTE4, 1, 1, 0 });
	}


	TDE2_API IFramePostProcessor* CreateFramePostProcessor(const TFramePostProcessorParameters& desc, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IFramePostProcessor, CFramePostProcessor, result, desc);
	}
}