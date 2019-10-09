#include "./../../include/graphics/CBaseRenderTarget.h"


namespace TDEngine2
{
	CBaseRenderTarget::CBaseRenderTarget() :
		CBaseResource()
	{
	}
	
	E_RESULT_CODE CBaseRenderTarget::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, 
										  const TTexture2DParameters& params)
	{
		E_RESULT_CODE result = _init(pResourceManager, name);

		if (result != RC_OK)
		{
			return result;
		}

		if (!pGraphicsContext)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsContext = pGraphicsContext;

		mWidth           = params.mWidth;
		mHeight          = params.mHeight;
		mFormat          = params.mFormat;
		mNumOfMipLevels  = params.mNumOfMipLevels;
		mNumOfSamples    = params.mNumOfSamples;
		mSamplingQuality = params.mSamplingQuality;

		mIsInitialized = true;

		return _createInternalTextureHandler(mpGraphicsContext, mWidth, mHeight, mFormat,
											 mNumOfMipLevels, mNumOfSamples, mSamplingQuality); /// create a texture's object within video memory using GAPI
	}

	U32 CBaseRenderTarget::GetWidth() const
	{
		return mWidth;
	}

	U32 CBaseRenderTarget::GetHeight() const
	{
		return mHeight;
	}

	E_FORMAT_TYPE CBaseRenderTarget::GetFormat() const
	{
		return mFormat;
	}
}