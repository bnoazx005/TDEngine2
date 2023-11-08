#include "../include/CVulkanTexture2D.h"
#include "../include/CVulkanMappings.h"
#include "../include/CVulkanGraphicsContext.h"
#include "../include/CVulkanUtils.h"
#include <core/IResourceManager.h>
#include <utils/Utils.h>


namespace TDEngine2
{
	CVulkanTexture2D::CVulkanTexture2D() :
		CBaseTexture2D()
	{
	}

	void CVulkanTexture2D::Bind(U32 slot)
	{
		CBaseTexture2D::Bind(slot);

		/*GL_SAFE_VOID_CALL(glActiveTexture(GL_TEXTURE0 + slot));
		GL_SAFE_VOID_CALL(glBindTexture(GL_TEXTURE_2D, mTextureHandler));*/
	}

	E_RESULT_CODE CVulkanTexture2D::Load()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		auto&& pResourceLoader = mpResourceManager->GetResourceLoader<ITexture2D>();

		if (!pResourceLoader)
		{
			return RC_FAIL;
		}

		return pResourceLoader->LoadResource(this);
	}

	E_RESULT_CODE CVulkanTexture2D::Unload()
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CVulkanTexture2D::Reset()
	{
		mIsInitialized = false;

		//GL_SAFE_CALL(glDeleteTextures(1, &mTextureHandler));
		//
		//mTextureHandler = 0;

		return RC_OK;
	}

	E_RESULT_CODE CVulkanTexture2D::WriteData(const TRectI32& regionRect, const U8* pData)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		//GL_SAFE_CALL(glBindTexture(GL_TEXTURE_2D, mTextureHandler));

		//GL_SAFE_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

		///// GL_UNSIGNED_BYTE is used explicitly, because of stb_image stores data as unsigned char array
		//GL_SAFE_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, regionRect.x, regionRect.y, regionRect.width, regionRect.height, 
		//							 CVulkanMappings::GetPixelDataFormat(mFormat), GL_UNSIGNED_BYTE, pData));

		//if (mNumOfMipLevels > 1)
		//{
		//	GL_SAFE_CALL(glGenerateMipmap(GL_TEXTURE_2D));
		//}

		//GL_SAFE_CALL(glBindTexture(GL_TEXTURE_2D, 0));

		return RC_OK;
	}

	std::vector<U8> CVulkanTexture2D::GetInternalData()
	{
		//std::vector<U8> pPixelData(mWidth * mHeight * CVulkanMappings::GetFormatSize(mFormat));

		/*Bind(0);
		GL_SAFE_VOID_CALL(glGetTexImage(GL_TEXTURE_2D, 0, CVulkanMappings::GetPixelDataFormat(mFormat), CVulkanMappings::GetBaseTypeOfFormat(mFormat), reinterpret_cast<void*>(&pPixelData[0])));
		
		if (CVulkanMappings::GetErrorCode(glGetError()) != RC_OK)
		{
			return {};
		}

		glBindTexture(GL_TEXTURE_2D, 0);*/

		//return std::move(pPixelData);
		return {};
	}

	E_RESULT_CODE CVulkanTexture2D::_createInternalTextureHandler(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
																U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality, bool isWriteable)
	{
		mpGraphicsContextImpl = dynamic_cast<CVulkanGraphicsContext*>(pGraphicsContext);

		/*GL_SAFE_CALL(glGenTextures(1, &mTextureHandler));

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
		GL_SAFE_CALL(glTexImage2D(GL_TEXTURE_2D, 0, CVulkanMappings::GetInternalFormat(format), width, height, 0,
								  CVulkanMappings::GetPixelDataFormat(format), GL_UNSIGNED_BYTE, nullptr));
				
		if (mipLevelsCount > 1)
		{
			GL_SAFE_CALL(glGenerateMipmap(GL_TEXTURE_2D));
		}

		if (isWriteable)
		{
			/// \todo Refactor this later
			GL_SAFE_CALL(glBindImageTexture(0, mTextureHandler, 0, GL_FALSE, 0, GL_READ_WRITE, CVulkanMappings::GetInternalFormat(format)));
		}

		glBindTexture(GL_TEXTURE_2D, 0);*/

		return RC_OK;
	}
	

	TDE2_API ITexture2D* CreateVulkanTexture2D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ITexture2D, CVulkanTexture2D, result, pResourceManager, pGraphicsContext, name);
	}

	TDE2_API ITexture2D* CreateVulkanTexture2D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
											const TTexture2DParameters& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ITexture2D, CVulkanTexture2D, result, pResourceManager, pGraphicsContext, name, params);
	}


	CVulkanTexture2DFactory::CVulkanTexture2DFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CVulkanTexture2DFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
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

	IResource* CVulkanTexture2DFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TTexture2DParameters& texParams = static_cast<const TTexture2DParameters&>(params);

		return dynamic_cast<IResource*>(CreateVulkanTexture2D(mpResourceManager, mpGraphicsContext, name, texParams, result));
	}

	IResource* CVulkanTexture2DFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		static const TTexture2DParameters defaultTextureParams{ 2, 2, FT_NORM_UBYTE4, 1, 1, 0 };

		TTexture2DParameters currParams = defaultTextureParams;
		currParams.mLoadingPolicy = params.mLoadingPolicy;

		// create blank texture, which sizes equals to 2 x 2 pixels of RGBA format
		return dynamic_cast<IResource*>(CreateVulkanTexture2D(mpResourceManager, mpGraphicsContext, name, currParams, result));
	}

	TypeId CVulkanTexture2DFactory::GetResourceTypeId() const
	{
		return ITexture2D::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateVulkanTexture2DFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CVulkanTexture2DFactory, result, pResourceManager, pGraphicsContext);
	}
}