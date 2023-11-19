#include "../include/CVulkanRenderTarget.h"
#include "../include/CVulkanMappings.h"
#include "../include/CVulkanUtils.h"
#include "../include/CVulkanTexture.h"
#include <utils/CFileLogger.h>
#include <core/IResourceManager.h>


namespace TDEngine2
{
	CVulkanRenderTarget::CVulkanRenderTarget() :
		CBaseRenderTarget()
	{
	}

	E_RESULT_CODE CVulkanRenderTarget::Load()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		auto&& pResourceLoader = mpResourceManager->GetResourceLoader<IRenderTarget>();

		if (!pResourceLoader)
		{
			return RC_FAIL;
		}

		return pResourceLoader->LoadResource(this);
	}

	E_RESULT_CODE CVulkanRenderTarget::Unload()
	{
		return Reset();
	}

	E_RESULT_CODE CVulkanRenderTarget::Reset()
	{
		mIsInitialized = false;

		/*GL_SAFE_CALL(glDeleteTextures(1, &mTextureHandler));
		GL_SAFE_CALL(glDeleteFramebuffers(1, &mTempFrameBufferHandler));

		mTextureHandler = 0;
		mTempFrameBufferHandler = 0;*/

		return RC_OK;
	}
	
	void CVulkanRenderTarget::Bind(U32 slot)
	{
		CBaseRenderTarget::Bind(slot);

		/*GL_SAFE_VOID_CALL(glActiveTexture(GL_TEXTURE0 + slot));
		GL_SAFE_VOID_CALL(glBindTexture(GL_TEXTURE_2D, mTextureHandler));*/
	}

	void CVulkanRenderTarget::UnbindFromShader()
	{
		//GL_SAFE_VOID_CALL(glActiveTexture(GL_TEXTURE0 + mLastBindingSlot));
		//GL_SAFE_VOID_CALL(glBindTexture(GL_TEXTURE_2D, 0));
	}
	
	E_RESULT_CODE CVulkanRenderTarget::Blit(ITexture2D*& pDestTexture)
	{
		if (!pDestTexture)
		{
			return RC_INVALID_ARGS;
		}

		//GL_SAFE_CALL(glBindFramebuffer(GL_FRAMEBUFFER, mTempFrameBufferHandler));
		//glViewport(0, 0, mWidth, mHeight);

		//pDestTexture->Bind(0);

		//GL_SAFE_CALL(glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, 0, 0, mWidth, mHeight, 0));

		//GL_SAFE_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

		return RC_OK;
	}
		
	E_RESULT_CODE CVulkanRenderTarget::_createInternalTextureHandler(IGraphicsContext* pGraphicsContext, const TRenderTargetParameters& params)
	{
		//GL_SAFE_CALL(glGenTextures(1, &mTextureHandler));

		//GL_SAFE_CALL(glBindTexture(GL_TEXTURE_2D, mTextureHandler));

		//GL_SAFE_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0));
		//GL_SAFE_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, params.mNumOfMipLevels));
		//GL_SAFE_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_NEVER));
		//GL_SAFE_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE));

		//GL_SAFE_CALL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		//GL_SAFE_CALL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
		//GL_SAFE_CALL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		//GL_SAFE_CALL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

		///// GL_UNSIGNED_BYTE is used explicitly, because of stb_image stores data as unsigned char array
		//GL_SAFE_CALL(glTexImage2D(GL_TEXTURE_2D, 0, CVulkanMappings::GetInternalFormat(params.mFormat), params.mWidth, params.mHeight, 0,
		//						  CVulkanMappings::GetPixelDataFormat(params.mFormat), GL_UNSIGNED_BYTE, nullptr));

		//if (params.mNumOfMipLevels > 1)
		//{
		//	GL_SAFE_CALL(glGenerateMipmap(GL_TEXTURE_2D));
		//}

		//glBindTexture(GL_TEXTURE_2D, 0);

		//// \note Create a temporary FBO for blit operations
		//GL_SAFE_CALL(glGenFramebuffers(1, &mTempFrameBufferHandler));

		//GL_SAFE_CALL(glBindFramebuffer(GL_FRAMEBUFFER, mTempFrameBufferHandler));
		//GL_SAFE_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureHandler, 0));
		//GL_SAFE_CALL(glReadBuffer(GL_COLOR_ATTACHMENT0));

		//if (params.mIsWriteable)
		//{
		//	TDE2_UNIMPLEMENTED();
		//}

		//GL_SAFE_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

		return RC_OK;
	}


	TDE2_API IRenderTarget* CreateVulkanRenderTarget(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
												  const TRenderTargetParameters& params, E_RESULT_CODE& result)
	{
		CVulkanRenderTarget* pRenderTargetInstance = new (std::nothrow) CVulkanRenderTarget();

		if (!pRenderTargetInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pRenderTargetInstance->Init(pResourceManager, pGraphicsContext, name, params);

		if (result != RC_OK)
		{
			delete pRenderTargetInstance;

			pRenderTargetInstance = nullptr;
		}

		return pRenderTargetInstance;
	}


	CVulkanRenderTargetFactory::CVulkanRenderTargetFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CVulkanRenderTargetFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
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

	IResource* CVulkanRenderTargetFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TRenderTargetParameters& texParams = static_cast<const TRenderTargetParameters&>(params);

		return dynamic_cast<IResource*>(CreateVulkanRenderTarget(mpResourceManager, mpGraphicsContext, name, texParams, result));
	}

	IResource* CVulkanRenderTargetFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TRenderTargetParameters& texParams = static_cast<const TRenderTargetParameters&>(params);

		return dynamic_cast<IResource*>(CreateVulkanRenderTarget(mpResourceManager, mpGraphicsContext, name, texParams, result));
	}

	TypeId CVulkanRenderTargetFactory::GetResourceTypeId() const
	{
		return IRenderTarget::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateVulkanRenderTargetFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		CVulkanRenderTargetFactory* pRenderTargetFactoryInstance = new (std::nothrow) CVulkanRenderTargetFactory();

		if (!pRenderTargetFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pRenderTargetFactoryInstance->Init(pResourceManager, pGraphicsContext);

		if (result != RC_OK)
		{
			delete pRenderTargetFactoryInstance;

			pRenderTargetFactoryInstance = nullptr;
		}

		return pRenderTargetFactoryInstance;
	}
}