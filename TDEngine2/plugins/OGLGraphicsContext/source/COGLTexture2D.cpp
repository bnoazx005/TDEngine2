#include "./../include/COGLTexture2D.h"
#include "./../include/COGLMappings.h"
#include "./../include/COGLUtils.h"
#include <core/IResourceManager.h>
#include <utils/Utils.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma warning(push)
#pragma warning(disable:4996)
#include "./../deps/stb/stb_image_write.h"
#pragma warning(pop)


namespace TDEngine2
{
	COGLTexture2D::COGLTexture2D() :
		CBaseTexture2D(), mTextureHandler(0)
	{
	}

	void COGLTexture2D::Bind(U32 slot)
	{
		GL_SAFE_VOID_CALL(glActiveTexture(GL_TEXTURE0 + slot));

		GL_SAFE_VOID_CALL(glBindTexture(GL_TEXTURE_2D, mTextureHandler));
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

		GL_SAFE_CALL(glDeleteTextures(1, &mTextureHandler));
		
		mTextureHandler = 0;

		return RC_OK;
	}

	E_RESULT_CODE COGLTexture2D::WriteData(const TRectI32& regionRect, const U8* pData)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		GL_SAFE_CALL(glBindTexture(GL_TEXTURE_2D, mTextureHandler));

		/// GL_UNSIGNED_BYTE is used explicitly, because of stb_image stores data as unsigned char array
		GL_SAFE_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, regionRect.x, regionRect.y, regionRect.width, regionRect.height, 
									 COGLMappings::GetPixelDataFormat(mFormat), GL_UNSIGNED_BYTE, pData));

		if (mNumOfMipLevels > 1)
		{
			GL_SAFE_CALL(glGenerateMipmap(GL_TEXTURE_2D));
		}

		glBindTexture(GL_TEXTURE_2D, 0);

		return RC_OK;
	}

	std::unique_ptr<U8[]> COGLTexture2D::GetInternalData()
	{
		std::unique_ptr<U8[]> pPixelData { new U8[mWidth * mHeight * COGLMappings::GetFormatSize(mFormat)] };

		glBindTexture(GL_TEXTURE_2D, mTextureHandler);
		glGetTexImage(GL_TEXTURE_2D, 0, COGLMappings::GetPixelDataFormat(mFormat), GL_UNSIGNED_BYTE, reinterpret_cast<void*>(pPixelData.get()));
		
		if (COGLMappings::GetErrorCode(glGetError()) != RC_OK)
		{
			return nullptr;
		}

		glBindTexture(GL_TEXTURE_2D, 0);

		return std::move(pPixelData);
	}

	E_RESULT_CODE COGLTexture2D::_createInternalTextureHandler(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
		U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality)
	{
		GL_SAFE_CALL(glGenTextures(1, &mTextureHandler));

		GL_SAFE_CALL(glBindTexture(GL_TEXTURE_2D, mTextureHandler));

		GL_SAFE_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0));
		GL_SAFE_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipLevelsCount));
		GL_SAFE_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_NEVER)); 
		GL_SAFE_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE)); 

		GL_SAFE_CALL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GL_SAFE_CALL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
		GL_SAFE_CALL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GL_SAFE_CALL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
				
		/// GL_UNSIGNED_BYTE is used explicitly, because of stb_image stores data as unsigned char array
		GL_SAFE_CALL(glTexImage2D(GL_TEXTURE_2D, 0, COGLMappings::GetInternalFormat(format), width, height, 0,
								  COGLMappings::GetPixelDataFormat(format), GL_UNSIGNED_BYTE, nullptr));
				
		if (mipLevelsCount > 1)
		{
			GL_SAFE_CALL(glGenerateMipmap(GL_TEXTURE_2D));
		}

		glBindTexture(GL_TEXTURE_2D, 0);

		return RC_OK;
	}
	

	TDE2_API ITexture2D* CreateOGLTexture2D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		COGLTexture2D* pTexture2DInstance = new (std::nothrow) COGLTexture2D();

		if (!pTexture2DInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pTexture2DInstance->Init(pResourceManager, pGraphicsContext, name);

		if (result != RC_OK)
		{
			delete pTexture2DInstance;

			pTexture2DInstance = nullptr;
		}

		return pTexture2DInstance;
	}

	TDE2_API ITexture2D* CreateOGLTexture2D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
											const TTexture2DParameters& params, E_RESULT_CODE& result)
	{
		COGLTexture2D* pTexture2DInstance = new (std::nothrow) COGLTexture2D();

		if (!pTexture2DInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pTexture2DInstance->Init(pResourceManager, pGraphicsContext, name, params);

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

	E_RESULT_CODE COGLTexture2DFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsContext || !pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;

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

	IResource* COGLTexture2DFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TTexture2DParameters& texParams = static_cast<const TTexture2DParameters&>(params);

		return dynamic_cast<IResource*>(CreateOGLTexture2D(mpResourceManager, mpGraphicsContext, name, texParams, result));
	}

	IResource* COGLTexture2DFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		// create blank texture, which sizes equals to 2 x 2 pixels of RGBA format
		return dynamic_cast<IResource*>(CreateOGLTexture2D(mpResourceManager, mpGraphicsContext, name, { 2, 2, FT_NORM_UBYTE4, 1, 1, 0 }, result));
	}

	U32 COGLTexture2DFactory::GetResourceTypeId() const
	{
		return CBaseTexture2D::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateOGLTexture2DFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		COGLTexture2DFactory* pTexture2DFactoryInstance = new (std::nothrow) COGLTexture2DFactory();

		if (!pTexture2DFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pTexture2DFactoryInstance->Init(pResourceManager, pGraphicsContext);

		if (result != RC_OK)
		{
			delete pTexture2DFactoryInstance;

			pTexture2DFactoryInstance = nullptr;
		}

		return pTexture2DFactoryInstance;
	}
}