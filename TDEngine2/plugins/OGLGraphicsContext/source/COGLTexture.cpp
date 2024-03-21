#include "../include/COGLTexture.h"
#include "../include/COGLMappings.h"
#include "../include/COGLUtils.h"
#include <core/IResourceManager.h>
#include <utils/Utils.h>


namespace TDEngine2
{
	static TResult<GLuint> CreateTextureResourceInternal(const TInitTextureImplParams& params)
	{
		GLuint textureHandle = 0;

		GL_SAFE_TRESULT_CALL(glGenTextures(1, &textureHandle));

		const GLenum textureType = COGLMappings::GetTextureType(params.mType);

		GL_SAFE_TRESULT_CALL(glBindTexture(textureType, textureHandle));

		GL_SAFE_TRESULT_CALL(glTexParameteri(textureType, GL_TEXTURE_BASE_LEVEL, 0));
		GL_SAFE_TRESULT_CALL(glTexParameteri(textureType, GL_TEXTURE_MAX_LEVEL, params.mNumOfMipLevels));
		GL_SAFE_TRESULT_CALL(glTexParameteri(textureType, GL_TEXTURE_COMPARE_FUNC, GL_NEVER));
		GL_SAFE_TRESULT_CALL(glTexParameteri(textureType, GL_TEXTURE_COMPARE_MODE, GL_NONE));

		GL_SAFE_TRESULT_CALL(glTexParameterf(textureType, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GL_SAFE_TRESULT_CALL(glTexParameterf(textureType, GL_TEXTURE_WRAP_T, GL_REPEAT));
		GL_SAFE_TRESULT_CALL(glTexParameterf(textureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GL_SAFE_TRESULT_CALL(glTexParameterf(textureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

		/// GL_UNSIGNED_BYTE is used explicitly, because of stb_image stores data as unsigned char array
		switch (params.mType)
		{
			case E_TEXTURE_IMPL_TYPE::TEXTURE_2D:
				GL_SAFE_TRESULT_CALL(glTexImage2D(textureType, 0, COGLMappings::GetInternalFormat(params.mFormat), params.mWidth, params.mHeight, 0,
					COGLMappings::GetPixelDataFormat(params.mFormat), GL_UNSIGNED_BYTE, nullptr));
				break;

			case E_TEXTURE_IMPL_TYPE::TEXTURE_2D_ARRAY:
			case E_TEXTURE_IMPL_TYPE::TEXTURE_3D:
				GL_SAFE_TRESULT_CALL(glTexImage3D(textureType, 0, COGLMappings::GetInternalFormat(params.mFormat),
					params.mWidth, params.mHeight, E_TEXTURE_IMPL_TYPE::TEXTURE_3D == params.mType ? params.mDepth : params.mArraySize, 0,
					COGLMappings::GetPixelDataFormat(params.mFormat),
					GL_UNSIGNED_BYTE, nullptr));
				break;

			case E_TEXTURE_IMPL_TYPE::CUBEMAP:
				for (U8 i = 0; i < 6; ++i)
				{
					GL_SAFE_TRESULT_CALL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, COGLMappings::GetInternalFormat(params.mFormat), params.mWidth, params.mHeight, 0,
						COGLMappings::GetPixelDataFormat(params.mFormat), GL_UNSIGNED_BYTE, nullptr));
				}
				break;
		}

		if (params.mNumOfMipLevels > 1)
		{
			GL_SAFE_TRESULT_CALL(glGenerateMipmap(textureType));
		}

		if (E_BIND_GRAPHICS_TYPE::BIND_UNORDERED_ACCESS == (params.mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_UNORDERED_ACCESS) || params.mIsWriteable)
		{
			/// \todo Refactor this later
			GL_SAFE_TRESULT_CALL(glBindImageTexture(0, textureHandle, 0, GL_FALSE, 0, GL_READ_WRITE, COGLMappings::GetInternalFormat(params.mFormat)));
		}

#if TDE2_DEBUG_MODE
		if (GLEW_KHR_debug)
		{
			glObjectLabel(GL_TEXTURE, textureHandle, -1, params.mName.c_str());
		}
#endif

		glBindTexture(textureType, 0);

		return Wrench::TOkValue<GLuint>(textureHandle);
	}


	COGLTextureImpl::COGLTextureImpl() :
		CBaseObject()
	{
	}

	E_RESULT_CODE COGLTextureImpl::Init(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params)
	{
		if (!pGraphicsContext ||
			!params.mWidth ||
			!params.mHeight ||
			params.mDepth < 1 ||
			params.mNumOfMipLevels < 1 ||
			!params.mArraySize)
		{
			return RC_INVALID_ARGS;
		}

		mInitParams = params;

		E_RESULT_CODE result = _onInitInternal();
		if (RC_OK != result)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE COGLTextureImpl::Resize(U32 width, U32 height, U32 depth)
	{
		E_RESULT_CODE result = _onFreeInternal();

		mInitParams.mWidth = width;
		mInitParams.mHeight = height;
		mInitParams.mDepth = depth;

		return _onInitInternal();
	}

	GLuint COGLTextureImpl::GetTextureHandle()
	{
		return mTextureHandle;
	}

	std::vector<U8> COGLTextureImpl::ReadBytes(U32 index)
	{
		std::vector<U8> pPixelData(mInitParams.mWidth * mInitParams.mHeight * COGLMappings::GetFormatSize(mInitParams.mFormat));

		const GLenum textureType = COGLMappings::GetTextureType(mInitParams.mType);

		GL_SAFE_VOID_CALL(glGetTexImage(textureType, index,
			COGLMappings::GetPixelDataFormat(mInitParams.mFormat), 
			COGLMappings::GetBaseTypeOfFormat(mInitParams.mFormat), 
			reinterpret_cast<void*>(&pPixelData[0])));
		
		if (COGLMappings::GetErrorCode(glGetError()) != RC_OK)
		{
			TDE2_ASSERT(false);
			return {};
		}
		
		glBindTexture(textureType, 0);
		
		return std::move(pPixelData);
	}

	const TInitTextureParams& COGLTextureImpl::GetParams() const
	{
		return mInitParams;
	}

	E_RESULT_CODE COGLTextureImpl::_onInitInternal()
	{
		auto createResourceResult = CreateTextureResourceInternal(mInitParams);
		if (createResourceResult.HasError())
		{
			return createResourceResult.GetError();
		}

		mTextureHandle = createResourceResult.Get();

		return RC_OK;
	}

	E_RESULT_CODE COGLTextureImpl::_onFreeInternal()
	{
		GL_SAFE_CALL(glDeleteTextures(1, &mTextureHandle));

		return RC_OK;
	}


	TDE2_API ITextureImpl* CreateOGLTextureImpl(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ITextureImpl, COGLTextureImpl, result, pGraphicsContext, params);
	}
}