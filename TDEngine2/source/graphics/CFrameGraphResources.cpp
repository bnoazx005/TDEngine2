#include "../../include/graphics/CFrameGraphResources.h"
#include "../../include/graphics/IGraphicsObjectManager.h"


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
		E_RESULT_CODE result = pGraphicsObjectManager->DestroyTexture(mTextureHandle);
		TDE2_ASSERT(RC_OK == result);

		mTextureHandle = TTextureHandleId::Invalid;

		return result;
	}

	void TFrameGraphTexture::BeforeReadOp()
	{
	}

	void TFrameGraphTexture::BeforeWriteOp()
	{
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
		E_RESULT_CODE result = pGraphicsObjectManager->DestroyBuffer(mBufferHandle);
		TDE2_ASSERT(RC_OK == result);

		mBufferHandle = TBufferHandleId::Invalid;

		return result;
	}

	void TFrameGraphBuffer::BeforeReadOp()
	{
	}

	void TFrameGraphBuffer::BeforeWriteOp()
	{
	}
}