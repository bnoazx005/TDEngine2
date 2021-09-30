#include "./../include/COGLCubemapTexture.h"
#include "./../include/COGLGraphicsContext.h"
#include "./../include/COGLMappings.h"
#include "./../include/COGLUtils.h"
#include <utils/Utils.h>
#include <cstring>
#include <algorithm>


namespace TDEngine2
{
	COGLCubemapTexture::COGLCubemapTexture() :
		CBaseCubemapTexture()
	{
	}

	void COGLCubemapTexture::Bind(U32 slot)
	{
		CBaseCubemapTexture::Bind(slot);

		GL_SAFE_VOID_CALL(glActiveTexture(GL_TEXTURE0 + slot));
		GL_SAFE_VOID_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureHandler));
	}

	E_RESULT_CODE COGLCubemapTexture::Reset()
	{
		mIsInitialized = false;
		
		GL_SAFE_CALL(glDeleteTextures(1, &mTextureHandler));
		
		mTextureHandler = 0;

		return RC_OK;
	}

	E_RESULT_CODE COGLCubemapTexture::WriteData(E_CUBEMAP_FACE face, const TRectI32& regionRect, const U8* pData)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		GL_SAFE_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureHandler));

		GL_SAFE_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

		/// GL_UNSIGNED_BYTE is used explicitly, because of stb_image stores data as unsigned char array
		GL_SAFE_CALL(glTexSubImage2D(COGLMappings::GetCubemapFace(face), 0, regionRect.x, regionRect.y, regionRect.width, regionRect.height,
									 COGLMappings::GetPixelDataFormat(mFormat), GL_UNSIGNED_BYTE, pData));

		if (mNumOfMipLevels > 1)
		{
			GL_SAFE_CALL(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));
		}

		GL_SAFE_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));

		return RC_OK;
	}

	E_RESULT_CODE COGLCubemapTexture::_createInternalTextureHandler(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
		U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality)
	{
		GL_SAFE_CALL(glGenTextures(1, &mTextureHandler));

		GL_SAFE_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureHandler));

		GL_SAFE_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0));
		GL_SAFE_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, mipLevelsCount));
		GL_SAFE_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_NEVER));
		GL_SAFE_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_NONE));

		GL_SAFE_CALL(glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GL_SAFE_CALL(glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT));
		GL_SAFE_CALL(glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT));
		GL_SAFE_CALL(glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GL_SAFE_CALL(glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

		/// GL_UNSIGNED_BYTE is used explicitly, because of stb_image stores data as unsigned char array
		for (U8 i = 0; i < 6; ++i)
		{
			GL_SAFE_CALL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, COGLMappings::GetInternalFormat(format), width, height, 0,
									  COGLMappings::GetPixelDataFormat(format), GL_UNSIGNED_BYTE, nullptr));
		}


		if (mipLevelsCount > 1)
		{
			GL_SAFE_CALL(glGenerateMipmap(GL_TEXTURE_2D));
		}

		GL_SAFE_CALL(glBindTexture(GL_TEXTURE_2D, 0));

		return RC_OK;
	}


	TDE2_API ICubemapTexture* CreateOGLCubemapTexture(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		COGLCubemapTexture* pCubemapTextureInstance = new (std::nothrow) COGLCubemapTexture();

		if (!pCubemapTextureInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pCubemapTextureInstance->Init(pResourceManager, pGraphicsContext, name);

		if (result != RC_OK)
		{
			delete pCubemapTextureInstance;

			pCubemapTextureInstance = nullptr;
		}

		return pCubemapTextureInstance;
	}


	TDE2_API ICubemapTexture* CreateOGLCubemapTexture(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
													  const TTexture2DParameters& params, E_RESULT_CODE& result)
	{
		COGLCubemapTexture* pTexture2DInstance = new (std::nothrow) COGLCubemapTexture();

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


	COGLCubemapTextureFactory::COGLCubemapTextureFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE COGLCubemapTextureFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
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

	IResource* COGLCubemapTextureFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;
		
		const TTexture2DParameters& texParams = static_cast<const TTexture2DParameters&>(params);

		return dynamic_cast<IResource*>(CreateOGLCubemapTexture(mpResourceManager, mpGraphicsContext, name, texParams, result));
	}

	IResource* COGLCubemapTextureFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		static const TTexture2DParameters defaultTextureParams{ 2, 2, FT_NORM_UBYTE4, 1, 1, 0 };

		TTexture2DParameters currParams = defaultTextureParams;
		currParams.mLoadingPolicy = params.mLoadingPolicy;

		// create blank texture, which sizes equals to 2 x 2 pixels of RGBA format
		return dynamic_cast<IResource*>(CreateOGLCubemapTexture(mpResourceManager, mpGraphicsContext, name, currParams, result));
	}

	TypeId COGLCubemapTextureFactory::GetResourceTypeId() const
	{
		return ICubemapTexture::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateOGLCubemapTextureFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		COGLCubemapTextureFactory* pCubemapTextureFactoryInstance = new (std::nothrow) COGLCubemapTextureFactory();

		if (!pCubemapTextureFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pCubemapTextureFactoryInstance->Init(pResourceManager, pGraphicsContext);

		if (result != RC_OK)
		{
			delete pCubemapTextureFactoryInstance;

			pCubemapTextureFactoryInstance = nullptr;
		}

		return pCubemapTextureFactoryInstance;
	}
}