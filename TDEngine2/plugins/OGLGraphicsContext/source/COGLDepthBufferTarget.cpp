#include "../include/COGLDepthBufferTarget.h"
#include "../include/COGLGraphicsContext.h"
#include "../include/COGLMappings.h"
#include "../include/COGLUtils.h"
#include "../include/COGLTexture2D.h"
#include <graphics/ITexture2D.h>
#include <cstring>


namespace TDEngine2
{
	COGLDepthBufferTarget::COGLDepthBufferTarget() :
		CBaseDepthBufferTarget()
	{
	}

	E_RESULT_CODE COGLDepthBufferTarget::Load()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		auto&& pResourceLoader = mpResourceManager->GetResourceLoader<IDepthBufferTarget>();

		if (!pResourceLoader)
		{
			return RC_FAIL;
		}

		return pResourceLoader->LoadResource(this);
	}

	E_RESULT_CODE COGLDepthBufferTarget::Unload()
	{
		return Reset();
	}

	E_RESULT_CODE COGLDepthBufferTarget::Reset()
	{
		mIsInitialized = false;

		GL_SAFE_CALL(glDeleteTextures(1, &mDepthBufferHandle));

		mDepthBufferHandle = 0;

		return RC_OK;
	}

	void COGLDepthBufferTarget::Bind(U32 slot)
	{
		CBaseDepthBufferTarget::Bind(slot);

		GL_SAFE_VOID_CALL(glActiveTexture(GL_TEXTURE0 + slot));
		GL_SAFE_VOID_CALL(glBindTexture(mIsCreatedAsCubemap ? GL_TEXTURE_CUBE_MAP : (mArraySize > 1 ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D), mDepthBufferHandle));
	}

	void COGLDepthBufferTarget::UnbindFromShader()
	{
		GL_SAFE_VOID_CALL(glActiveTexture(GL_TEXTURE0 + mLastBindingSlot));
		GL_SAFE_VOID_CALL(glBindTexture(GL_TEXTURE_2D, 0));
	}

	E_RESULT_CODE COGLDepthBufferTarget::Blit(ITexture2D*& pDestTexture)
	{
		if (!pDestTexture)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}

	GLuint COGLDepthBufferTarget::GetInternalHandler() const
	{
		return mDepthBufferHandle;
	}

	E_RESULT_CODE COGLDepthBufferTarget::_createInternalTextureHandler(IGraphicsContext* pGraphicsContext, const TRenderTargetParameters& params)
	{
		GL_SAFE_CALL(glGenTextures(1, &mDepthBufferHandle));

		GLenum targetType = GL_TEXTURE_2D;

		switch (params.mType)
		{
			case TRenderTargetParameters::E_TARGET_TYPE::CUBEMAP:
				targetType = GL_TEXTURE_CUBE_MAP;
				break;
			case TRenderTargetParameters::E_TARGET_TYPE::TEXTURE2D_ARRAY:
				targetType = GL_TEXTURE_2D_ARRAY;
				break;
		}

		GL_SAFE_CALL(glBindTexture(targetType, mDepthBufferHandle));

		GL_SAFE_CALL(glTexParameteri(targetType, GL_TEXTURE_BASE_LEVEL, 0));
		GL_SAFE_CALL(glTexParameteri(targetType, GL_TEXTURE_MAX_LEVEL, params.mNumOfMipLevels));
		GL_SAFE_CALL(glTexParameteri(targetType, GL_TEXTURE_COMPARE_FUNC, GL_NEVER));
		GL_SAFE_CALL(glTexParameteri(targetType, GL_TEXTURE_COMPARE_MODE, GL_NONE));

		GL_SAFE_CALL(glTexParameterf(targetType, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GL_SAFE_CALL(glTexParameterf(targetType, GL_TEXTURE_WRAP_T, GL_REPEAT));
		GL_SAFE_CALL(glTexParameterf(targetType, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GL_SAFE_CALL(glTexParameterf(targetType, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

		/// GL_UNSIGNED_BYTE is used explicitly, because of stb_image stores data as unsigned char array
		switch (params.mType)
		{
			case TRenderTargetParameters::E_TARGET_TYPE::TEXTURE2D:
				GL_SAFE_CALL(glTexImage2D(GL_TEXTURE_2D, 0, COGLMappings::GetInternalFormat(params.mFormat), 
					params.mWidth, params.mHeight, 0, 
					COGLMappings::GetPixelDataFormat(params.mFormat), 
					GL_UNSIGNED_BYTE, nullptr));
				break;

			case TRenderTargetParameters::E_TARGET_TYPE::TEXTURE2D_ARRAY:
				GL_SAFE_CALL(glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, COGLMappings::GetInternalFormat(params.mFormat),
					params.mWidth, params.mHeight, params.mArraySize, 0,
					COGLMappings::GetPixelDataFormat(params.mFormat),
					GL_UNSIGNED_BYTE, nullptr));

				mArraySize = params.mArraySize;
				break;

			case TRenderTargetParameters::E_TARGET_TYPE::CUBEMAP:
				for (U8 i = 0; i < 6; i++)
				{
					GL_SAFE_CALL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 
						COGLMappings::GetInternalFormat(params.mFormat), 
						params.mWidth, params.mHeight, 0, 
						COGLMappings::GetPixelDataFormat(params.mFormat), 
						GL_UNSIGNED_BYTE, nullptr));
				}
				break;
		}
		
		if (params.mIsWriteable)
		{
			TDE2_UNIMPLEMENTED();
		}

		glBindTexture(GL_TEXTURE_2D, 0);

		return RC_OK;
	}


	TDE2_API IDepthBufferTarget* CreateOGLDepthBufferTarget(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
															const TRenderTargetParameters& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IDepthBufferTarget, COGLDepthBufferTarget, result, pResourceManager, pGraphicsContext, name, params);
	}


	COGLDepthBufferTargetFactory::COGLDepthBufferTargetFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE COGLDepthBufferTargetFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
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

	IResource* COGLDepthBufferTargetFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TRenderTargetParameters& texParams = static_cast<const TRenderTargetParameters&>(params);

		return dynamic_cast<IResource*>(CreateOGLDepthBufferTarget(mpResourceManager, mpGraphicsContext, name, texParams, result));
	}

	IResource* COGLDepthBufferTargetFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TRenderTargetParameters& texParams = static_cast<const TRenderTargetParameters&>(params);

		return dynamic_cast<IResource*>(CreateOGLDepthBufferTarget(mpResourceManager, mpGraphicsContext, name, texParams, result));
	}

	TypeId COGLDepthBufferTargetFactory::GetResourceTypeId() const
	{
		return IDepthBufferTarget::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateOGLDepthBufferTargetFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, COGLDepthBufferTargetFactory, result, pResourceManager, pGraphicsContext);
	}
}
