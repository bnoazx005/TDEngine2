#include "../../include/graphics/CFrameGraphResources.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/core/IGraphicsContext.h"


namespace TDEngine2
{
	/*!
		TFrameGraphTexture's definition
	*/

	E_RESULT_CODE TFrameGraphTexture::Acquire(IGraphicsObjectManager* pGraphicsObjectManager, const TDesc& desc)
	{
		auto createTextureResult = pGraphicsObjectManager->CreateTexture(static_cast<const TInitTextureImplParams&>(desc));
		if (createTextureResult.IsOk())
		{
			mTextureHandle = createTextureResult.Get();
			return RC_OK;
		}

		return createTextureResult.GetError();
	}

	E_RESULT_CODE TFrameGraphTexture::Release(IGraphicsObjectManager* pGraphicsObjectManager)
	{
		if (TTextureHandleId::Invalid == mTextureHandle)
		{
			return RC_OK;
		}

		E_RESULT_CODE result = pGraphicsObjectManager->DestroyTexture(mTextureHandle);
		TDE2_ASSERT(RC_OK == result);

		mTextureHandle = TTextureHandleId::Invalid;

		return result;
	}

	void TFrameGraphTexture::BeforeReadOp(IGraphicsObjectManager* pGraphicsObjectManager, const TDesc& desc)
	{
		TPtr<ITextureImpl> pTexture = pGraphicsObjectManager->GetTexturePtr(mTextureHandle);
		if (!pTexture)
		{
			return;
		}

		if ((E_BIND_GRAPHICS_TYPE::BIND_SHADER_RESOURCE & desc.mBindFlags) == E_BIND_GRAPHICS_TYPE::BIND_SHADER_RESOURCE)
		{
			pTexture->Transition(E_RESOURCE_LAYOUT::SHADER_RESOURCE);
		}
	}

	void TFrameGraphTexture::BeforeWriteOp(IGraphicsObjectManager* pGraphicsObjectManager, const TDesc& desc)
	{
		TPtr<ITextureImpl> pTexture = pGraphicsObjectManager->GetTexturePtr(mTextureHandle);
		if (!pTexture)
		{
			return;
		}

		// \note for render/(depth/stencil) targets barriers specified within BeginRenderPass
		
		if ((E_BIND_GRAPHICS_TYPE::BIND_UNORDERED_ACCESS & desc.mBindFlags) == E_BIND_GRAPHICS_TYPE::BIND_UNORDERED_ACCESS)
		{
			pTexture->Transition(E_RESOURCE_LAYOUT::UAV_RESOURCE);
		}
	}


	/*!
		TFrameGraphBuffer's definition
	*/

	E_RESULT_CODE TFrameGraphBuffer::Acquire(IGraphicsObjectManager* pGraphicsObjectManager, const TDesc& desc)
	{
		auto createBufferResult = pGraphicsObjectManager->CreateBuffer(static_cast<const TInitBufferParams&>(desc));
		if (createBufferResult.IsOk())
		{
			mBufferHandle = createBufferResult.Get();
			return RC_OK;
		}

		return createBufferResult.GetError();
	}

	E_RESULT_CODE TFrameGraphBuffer::Release(IGraphicsObjectManager* pGraphicsObjectManager)
	{
		if (TBufferHandleId::Invalid == mBufferHandle)
		{
			return RC_OK;
		}

		E_RESULT_CODE result = pGraphicsObjectManager->DestroyBuffer(mBufferHandle);
		TDE2_ASSERT(RC_OK == result);

		mBufferHandle = TBufferHandleId::Invalid;

		return result;
	}

	void TFrameGraphBuffer::BeforeReadOp(IGraphicsObjectManager* pGraphicsObjectManager, const TDesc& desc)
	{
		TPtr<IBuffer> pBuffer = pGraphicsObjectManager->GetBufferPtr(mBufferHandle);

		TBufferTransitionBarrierInfo transitionInfo{};
		transitionInfo.mHandle     = mBufferHandle;
		transitionInfo.mCurrLayout = E_RESOURCE_LAYOUT::UNDEFINED;
		transitionInfo.mNewLayout  = E_RESOURCE_LAYOUT::SHADER_RESOURCE;

		pGraphicsObjectManager->GetGraphicsContext()->TransitionBarrier(transitionInfo);
	}

	void TFrameGraphBuffer::BeforeWriteOp(IGraphicsObjectManager* pGraphicsObjectManager, const TDesc& desc)
	{
		TPtr<IBuffer> pBuffer = pGraphicsObjectManager->GetBufferPtr(mBufferHandle);

		TBufferTransitionBarrierInfo transitionInfo{};
		transitionInfo.mHandle     = mBufferHandle;
		transitionInfo.mCurrLayout = E_RESOURCE_LAYOUT::UNDEFINED;
		transitionInfo.mNewLayout  = (E_STRUCTURED_BUFFER_TYPE::INDIRECT_DRAW_BUFFER == desc.mStructuredBufferType) ? E_RESOURCE_LAYOUT::INDIRECT_ARGS_BUFFER : E_RESOURCE_LAYOUT::UAV_RESOURCE;

		pGraphicsObjectManager->GetGraphicsContext()->TransitionBarrier(transitionInfo);
	}
}