#include "./../include/COGLTexture2D.h"
#include "./../include/COGLMappings.h"
#include <core/IResourceManager.h>


namespace TDEngine2
{
	COGLTexture2D::COGLTexture2D() :
		CBaseTexture2D(), mTextureHandler(0)
	{
	}

	E_RESULT_CODE COGLTexture2D::Load()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		const IResourceLoader* pResourceLoader = mpResourceManager->GetResourceLoader<CBaseTexture2D>();

		if (!pResourceLoader)
		{
			return RC_FAIL;
		}

		return pResourceLoader->LoadResource(this);
	}

	E_RESULT_CODE COGLTexture2D::Unload()
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE COGLTexture2D::Reset()
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

	E_RESULT_CODE COGLTexture2D::WriteData(const TRectI32& regionRect, const U8* pData)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		glBindTexture(GL_TEXTURE_2D, mTextureHandler);
		
		/// GL_UNSIGNED_BYTE is used explicitly, because of stb_image stores data as unsigned char array
		glTexSubImage2D(GL_TEXTURE_2D, 0, regionRect.left, regionRect.top, regionRect.right, regionRect.bottom, 
						COGLMappings::GetPixelDataFormat(mFormat), GL_UNSIGNED_BYTE, pData);

		if (glGetError() != GL_NO_ERROR)
		{
			return RC_FAIL;
		}

		glBindTexture(GL_TEXTURE_2D, 0);

		return RC_OK;
	}

	E_RESULT_CODE COGLTexture2D::_createInternalTextureHandler(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
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
	

	TDE2_API ITexture2D* CreateOGLTexture2D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
											TResourceId id, E_RESULT_CODE& result)
	{
		COGLTexture2D* pTexture2DInstance = new (std::nothrow) COGLTexture2D();

		if (!pTexture2DInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pTexture2DInstance->Init(pResourceManager, pGraphicsContext, name, id);

		if (result != RC_OK)
		{
			delete pTexture2DInstance;

			pTexture2DInstance = nullptr;
		}

		return pTexture2DInstance;
	}

	TDE2_API ITexture2D* CreateOGLTexture2D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
											TResourceId id, U32 width, U32 height, E_FORMAT_TYPE format, U32 mipLevelsCount,
											U32 samplesCount, U32 samplingQuality, E_RESULT_CODE& result)
	{
		COGLTexture2D* pTexture2DInstance = new (std::nothrow) COGLTexture2D();

		if (!pTexture2DInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pTexture2DInstance->Init(pResourceManager, pGraphicsContext, name, id, width, height, format,
			mipLevelsCount, samplesCount, samplingQuality);

		if (result != RC_OK)
		{
			delete pTexture2DInstance;

			pTexture2DInstance = nullptr;
		}

		return pTexture2DInstance;
	}


	COGLTexture2DFactory::COGLTexture2DFactory() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE COGLTexture2DFactory::Init(IGraphicsContext* pGraphicsContext)
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

	E_RESULT_CODE COGLTexture2DFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IResource* COGLTexture2DFactory::Create(const TBaseResourceParameters* pParams) const
	{
		E_RESULT_CODE result = RC_OK;

		const TTexture2DParameters* pTexParams = static_cast<const TTexture2DParameters*>(pParams);

		return dynamic_cast<IResource*>(CreateOGLTexture2D(pTexParams->mpResourceManager, pTexParams->mpGraphicsContext, pTexParams->mName, pTexParams->mId,
														   pTexParams->mWidth, pTexParams->mHeight, pTexParams->mFormat, pTexParams->mNumOfMipLevels,
														   pTexParams->mNumOfSamples, pTexParams->mSamplingQuality, result));
	}

	IResource* COGLTexture2DFactory::CreateDefault(const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		// create blank texture, which sizes equals to 2 x 2 pixels of RGBA format
		return dynamic_cast<IResource*>(CreateOGLTexture2D(params.mpResourceManager, mpGraphicsContext, params.mName, params.mId,
														   2, 2, FT_NORM_UBYTE4, 1, 1, 0, result));
	}

	U32 COGLTexture2DFactory::GetResourceTypeId() const
	{
		return CBaseTexture2D::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateOGLTexture2DFactory(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		COGLTexture2DFactory* pTexture2DFactoryInstance = new (std::nothrow) COGLTexture2DFactory();

		if (!pTexture2DFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pTexture2DFactoryInstance->Init(pGraphicsContext);

		if (result != RC_OK)
		{
			delete pTexture2DFactoryInstance;

			pTexture2DFactoryInstance = nullptr;
		}

		return pTexture2DFactoryInstance;
	}
}