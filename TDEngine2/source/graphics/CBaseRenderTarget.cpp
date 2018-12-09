#include "./../../include/graphics/CBaseRenderTarget.h"


namespace TDEngine2
{
	CBaseRenderTarget::CBaseRenderTarget() :
		CBaseResource()
	{
	}
	
	E_RESULT_CODE CBaseRenderTarget::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, TResourceId id,
										  U32 width, U32 height, E_FORMAT_TYPE format, U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality)
	{
		E_RESULT_CODE result = _init(pResourceManager, name, id);

		if (result != RC_OK)
		{
			return result;
		}

		if (!pGraphicsContext)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsContext = pGraphicsContext;

		mWidth = width;

		mHeight = height;

		mFormat = format;

		mNumOfMipLevels = mipLevelsCount;

		mNumOfSamples = samplesCount;

		mSamplingQuality = samplingQuality;

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
}