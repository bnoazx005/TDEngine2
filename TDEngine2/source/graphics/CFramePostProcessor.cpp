#include "../../include/graphics/CFramePostProcessor.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/core/IWindowSystem.h"
#include "../../include/graphics/CBaseRenderTarget.h"
#include "../../include/core/IGraphicsContext.h"


namespace TDEngine2
{
	CFramePostProcessor::CFramePostProcessor() :
		CBaseObject()
	{
	}


	E_RESULT_CODE CFramePostProcessor::Init(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, IWindowSystem* pWindowSystem)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pRenderer || !pGraphicsObjectManager || !pWindowSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pRenderer->GetResourceManager();

		mpOverlayRenderQueue              = pRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_OVERLAY);
		mDefaultScreenSpaceMaterialHandle = mpResourceManager->Create<CBaseMaterial>("DefaultScreenSpaceEffect.material", TMaterialParameters { "DefaultScreenSpaceEffect", false, { false, false } });
		mpWindowSystem                    = pWindowSystem;
		mpGraphicsContext                 = pGraphicsObjectManager->GetGraphicsContext();
		mRenderTargetHandle               = TResourceId::Invalid;

		if (auto vertexFormatResult = pGraphicsObjectManager->CreateVertexDeclaration())
		{
			mpVertexFormatDeclaration = vertexFormatResult.Get();
		}

		TDE2_ASSERT(mpVertexFormatDeclaration);
		mpVertexFormatDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION });

		const void* pScreenTriangleVerts = static_cast<const void*>(&pGraphicsObjectManager->GetScreenTriangleVertices()[0]);

		if (auto vertexBufferResult = pGraphicsObjectManager->CreateVertexBuffer(BUT_STATIC, 3 * sizeof(TVector4), pScreenTriangleVerts))
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

		if (mRenderTargetHandle == TResourceId::Invalid)
		{
			mRenderTargetHandle = _getRenderTarget(mpWindowSystem->GetWidth(), mpWindowSystem->GetHeight(), true);
			pCurrRenderTarget = dynamic_cast<IRenderTarget*>(mpResourceManager->GetResource(mRenderTargetHandle));

			if (auto pScreenSpaceMaterial = dynamic_cast<IMaterial*>(mpResourceManager->GetResource(mDefaultScreenSpaceMaterialHandle)))
			{
				pScreenSpaceMaterial->SetTextureResource("FrameTexture", pCurrRenderTarget);
			}
		}

		pCurrRenderTarget = GetValidPtrOrDefault(pCurrRenderTarget, dynamic_cast<IRenderTarget*>(mpResourceManager->GetResource(mRenderTargetHandle)));

		{
			mpGraphicsContext->BindRenderTarget(0, pCurrRenderTarget);
			onRenderFrameCallback();
			mpGraphicsContext->BindRenderTarget(0, nullptr);
		}

		_submitFullScreenTriangle(mpOverlayRenderQueue);

		return RC_OK;
	}

	E_RESULT_CODE CFramePostProcessor::PostRender()
	{
		TDE2_UNIMPLEMENTED();
		return RC_NOT_IMPLEMENTED_YET;
	}

	void CFramePostProcessor::_submitFullScreenTriangle(CRenderQueue* pRenderQueue)
	{
		// \note The fullscreen triangle is drawn without vertex buffer, all the geometry is generated inside the vertex shader
		TDrawCommandPtr pDrawCommand      = pRenderQueue->SubmitDrawCommand<TDrawCommand>(static_cast<U32>(E_GEOMETRY_SUBGROUP_TAGS::IMAGE_EFFECTS));
		pDrawCommand->mNumOfVertices      = 3;
		pDrawCommand->mPrimitiveType      = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
		pDrawCommand->mMaterialHandle     = mDefaultScreenSpaceMaterialHandle;
		pDrawCommand->mpVertexBuffer      = mpFullScreenTriangleVertexBuffer;
		pDrawCommand->mpVertexDeclaration = mpVertexFormatDeclaration;
	}

	TResourceId CFramePostProcessor::_getRenderTarget(U32 width, U32 height, bool isHDRSupport)
	{
		return mpResourceManager->Create<CBaseRenderTarget>("MainRenderTarget", TTexture2DParameters{ width, height, isHDRSupport ? FT_FLOAT4 : FT_NORM_UBYTE4, 1, 1, 0 });
	}


	TDE2_API IFramePostProcessor* CreateFramePostProcessor(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, IWindowSystem* pWindowSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IFramePostProcessor, CFramePostProcessor, result, pRenderer, pGraphicsObjectManager, pWindowSystem);
	}
}