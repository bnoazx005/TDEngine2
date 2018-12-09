#include "./../include/COGLRenderTarget.h"
#include "./../include/COGLMappings.h"
#include <core/IResourceManager.h>


namespace TDEngine2
{
	COGLRenderTarget::COGLRenderTarget() :
		CBaseRenderTarget(), mTextureHandler(0)
	{
	}

	E_RESULT_CODE COGLRenderTarget::Load()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		const IResourceLoader* pResourceLoader = mpResourceManager->GetResourceLoader<CBaseRenderTarget>();

		if (!pResourceLoader)
		{
			return RC_FAIL;
		}

		return pResourceLoader->LoadResource(this);
	}

	E_RESULT_CODE COGLRenderTarget::Unload()
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE COGLRenderTarget::Reset()
	{
		mIsInitialized = false;

		glDeleteTextures(1, &mTextureHandler);

		if (glGetError() != GL_NO_ERROR)
		{
			return RC_FAIL;
		}

		mTextureHandler = 0;

		return RC_OK;
	}
	
	E_RESULT_CODE COGLRenderTarget::_createInternalTextureHandler(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
																  U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality)
	{
		glGenTextures(1, &mTextureHandler);

		if (glGetError() != GL_NO_ERROR)
		{
			return RC_FAIL;
		}

		glBindTexture(GL_TEXTURE_2D, mTextureHandler);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, mipLevelsCount);

		/// GL_UNSIGNED_BYTE is used explicitly, because of stb_image stores data as unsigned char array
		glTexImage2D(GL_TEXTURE_2D, 0, COGLMappings::GetInternalFormat(format), width, height, 0,
					 COGLMappings::GetPixelDataFormat(format), GL_UNSIGNED_BYTE, nullptr);

		if (glGetError() != GL_NO_ERROR)
		{
			return RC_FAIL;
		}

		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);

		return RC_OK;
	}


	TDE2_API IRenderTarget* CreateOGLRenderTarget(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
												  TResourceId id, U32 width, U32 height, E_FORMAT_TYPE format, U32 mipLevelsCount,
												  U32 samplesCount, U32 samplingQuality, E_RESULT_CODE& result)
	{
		COGLRenderTarget* pRenderTargetInstance = new (std::nothrow) COGLRenderTarget();

		if (!pRenderTargetInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pRenderTargetInstance->Init(pResourceManager, pGraphicsContext, name, id, width, height, format,
											 mipLevelsCount, samplesCount, samplingQuality);

		if (result != RC_OK)
		{
			delete pRenderTargetInstance;

			pRenderTargetInstance = nullptr;
		}

		return pRenderTargetInstance;
	}


	COGLRenderTargetFactory::COGLRenderTargetFactory() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE COGLRenderTargetFactory::Init(IGraphicsContext* pGraphicsContext)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsContext)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsContext = pGraphicsContext;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE COGLRenderTargetFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IResource* COGLRenderTargetFactory::Create(const TBaseResourceParameters* pParams) const
	{
		E_RESULT_CODE result = RC_OK;

		const TRenderTargetParameters* pTexParams = static_cast<const TRenderTargetParameters*>(pParams);

		return dynamic_cast<IResource*>(CreateOGLRenderTarget(pTexParams->mpResourceManager, pTexParams->mpGraphicsContext, pTexParams->mName, pTexParams->mId,
															  pTexParams->mWidth, pTexParams->mHeight, pTexParams->mFormat, pTexParams->mNumOfMipLevels,
															  pTexParams->mNumOfSamples, pTexParams->mSamplingQuality, result));
	}

	IResource* COGLRenderTargetFactory::CreateDefault(const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;
		
		return nullptr;
	}

	U32 COGLRenderTargetFactory::GetResourceTypeId() const
	{
		return CBaseRenderTarget::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateOGLRenderTargetFactory(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		COGLRenderTargetFactory* pRenderTargetFactoryInstance = new (std::nothrow) COGLRenderTargetFactory();

		if (!pRenderTargetFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pRenderTargetFactoryInstance->Init(pGraphicsContext);

		if (result != RC_OK)
		{
			delete pRenderTargetFactoryInstance;

			pRenderTargetFactoryInstance = nullptr;
		}

		return pRenderTargetFactoryInstance;
	}
}