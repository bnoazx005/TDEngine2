#include "./../include/COGLRenderTarget.h"
#include "./../include/COGLMappings.h"
#include "./../include/COGLUtils.h"
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
		return Reset();
	}

	E_RESULT_CODE COGLRenderTarget::Reset()
	{
		mIsInitialized = false;

		GL_SAFE_CALL(glDeleteTextures(1, &mTextureHandler));
		GL_SAFE_CALL(glDeleteFramebuffers(1, &mFrameBufferHandler));

		mTextureHandler     = 0;
		mFrameBufferHandler = 0;

		return RC_OK;
	}
	
	void COGLRenderTarget::Bind(U32 slot)
	{
		CBaseRenderTarget::Bind(slot);

		glBindTexture(GL_TEXTURE_2D, mTextureHandler);
	}

	GLuint COGLRenderTarget::GetInternalHandler() const
	{
		return mFrameBufferHandler;
	}
	
	E_RESULT_CODE COGLRenderTarget::_createInternalTextureHandler(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
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

		return _createFrameBufferHandler();
	}

	E_RESULT_CODE COGLRenderTarget::_createFrameBufferHandler()
	{		
		GL_SAFE_CALL(glGenFramebuffers(1, &mFrameBufferHandler));

		GL_SAFE_CALL(glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferHandler));
		GL_SAFE_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureHandler, 0));
		GL_SAFE_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

		return RC_OK;
	}


	TDE2_API IRenderTarget* CreateOGLRenderTarget(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
												  const TTexture2DParameters& params, E_RESULT_CODE& result)
	{
		COGLRenderTarget* pRenderTargetInstance = new (std::nothrow) COGLRenderTarget();

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


	COGLRenderTargetFactory::COGLRenderTargetFactory() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE COGLRenderTargetFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
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

	IResource* COGLRenderTargetFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TTexture2DParameters& texParams = static_cast<const TTexture2DParameters&>(params);

		return dynamic_cast<IResource*>(CreateOGLRenderTarget(mpResourceManager, mpGraphicsContext, name, texParams, result));
	}

	IResource* COGLRenderTargetFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TTexture2DParameters& texParams = static_cast<const TTexture2DParameters&>(params);

		return dynamic_cast<IResource*>(CreateOGLRenderTarget(mpResourceManager, mpGraphicsContext, name, texParams, result));
	}

	U32 COGLRenderTargetFactory::GetResourceTypeId() const
	{
		return CBaseRenderTarget::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateOGLRenderTargetFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		COGLRenderTargetFactory* pRenderTargetFactoryInstance = new (std::nothrow) COGLRenderTargetFactory();

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