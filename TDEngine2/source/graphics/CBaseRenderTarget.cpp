#include "../../include/graphics/CBaseRenderTarget.h"
#include "../../include/graphics/CBaseTexture2D.h"
#include "../../include/core/IGraphicsContext.h"


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

		mWidth                = params.mWidth;
		mHeight               = params.mHeight;
		mFormat               = params.mFormat;
		mNumOfMipLevels       = params.mNumOfMipLevels;
		mNumOfSamples         = params.mNumOfSamples;
		mSamplingQuality      = params.mSamplingQuality;
		mTextureSamplerParams = params.mTexSamplerDesc;
		mIsRandomlyWriteable  = params.mIsWriteable; 

		mTextureSamplerParams.mUseMipMaps = mNumOfMipLevels > 1;

		mIsInitialized = true;

		return _createInternalTextureHandler(mpGraphicsContext, mWidth, mHeight, mFormat,
											 mNumOfMipLevels, mNumOfSamples, mSamplingQuality, mIsRandomlyWriteable); /// create a texture's object within video memory using GAPI
	}

	void CBaseRenderTarget::Bind(U32 slot)
	{
		if (mCurrTextureSamplerHandle == TTextureSamplerId::Invalid)
		{
			mCurrTextureSamplerHandle = CBaseTexture2D::GetTextureSampleHandle(mpGraphicsContext, mTextureSamplerParams);
		}

		mpGraphicsContext->BindTextureSampler(slot, mCurrTextureSamplerHandle);
	}

	E_RESULT_CODE CBaseRenderTarget::Resize(U32 width, U32 height)
	{
		E_RESULT_CODE result = Reset();
		mIsInitialized = true;

		mWidth  = width;
		mHeight = height;

		result = result | _createInternalTextureHandler(mpGraphicsContext, mWidth, mHeight, mFormat, mNumOfMipLevels, mNumOfSamples, mSamplingQuality, mIsRandomlyWriteable);

		return result;
	}

	void CBaseRenderTarget::SetUWrapMode(const E_ADDRESS_MODE_TYPE& mode)
	{
		mTextureSamplerParams.mUAddressMode = mode;
	}

	void CBaseRenderTarget::SetVWrapMode(const E_ADDRESS_MODE_TYPE& mode)
	{
		mTextureSamplerParams.mVAddressMode = mode;
	}

	void CBaseRenderTarget::SetWWrapMode(const E_ADDRESS_MODE_TYPE& mode)
	{
		mTextureSamplerParams.mWAddressMode = mode;
	}

	void CBaseRenderTarget::SetFilterType(const E_TEXTURE_FILTER_TYPE& type)
	{
		mTextureSamplerParams.mFilteringType = type;
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

	TRectF32 CBaseRenderTarget::GetNormalizedTextureRect() const
	{
		return { 0.0f, 0.0f, 1.0f, 1.0f };
	}

	const TPtr<IResourceLoader> CBaseRenderTarget::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<IRenderTarget>();
	}


	CBaseDepthBufferTarget::CBaseDepthBufferTarget() :
		CBaseRenderTarget()
	{
	}
}