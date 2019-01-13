#include "./../include/COGLGraphicsObjectManager.h"
#include "./../include/COGLVertexBuffer.h"
#include "./../include/COGLIndexBuffer.h"
#include "./../include/COGLConstantBuffer.h"
#include "./../include/COGLVertexDeclaration.h"
#include "./../include/COGLMappings.h"


namespace TDEngine2
{
	COGLGraphicsObjectManager::COGLGraphicsObjectManager() :
		CBaseGraphicsObjectManager()
	{
	}

	TResult<IVertexBuffer*> COGLGraphicsObjectManager::CreateVertexBuffer(E_BUFFER_USAGE_TYPE usageType, U32 totalBufferSize, void* pDataPtr)
	{
		E_RESULT_CODE result = RC_OK;

		IVertexBuffer* pNewVertexBuffer = CreateOGLVertexBuffer(mpGraphicsContext, usageType, totalBufferSize, pDataPtr, result);

		if (result != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		_insertBuffer(pNewVertexBuffer);

		return TOkValue<IVertexBuffer*>(pNewVertexBuffer);
	}

	TResult<IIndexBuffer*> COGLGraphicsObjectManager::CreateIndexBuffer(E_BUFFER_USAGE_TYPE usageType, E_INDEX_FORMAT_TYPE indexFormatType,
																		U32 totalBufferSize, void* pDataPtr)
	{
		E_RESULT_CODE result = RC_OK;

		IIndexBuffer* pNewIndexBuffer = CreateOGLIndexBuffer(mpGraphicsContext, usageType, indexFormatType, totalBufferSize, pDataPtr, result);

		if (result != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		_insertBuffer(pNewIndexBuffer);

		return TOkValue<IIndexBuffer*>(pNewIndexBuffer);
	}

	TResult<IConstantBuffer*> COGLGraphicsObjectManager::CreateConstantBuffer(E_BUFFER_USAGE_TYPE usageType, U32 totalBufferSize, void* pDataPtr)
	{
		E_RESULT_CODE result = RC_OK;

		IConstantBuffer* pNewConstantBuffer = CreateOGLConstantBuffer(mpGraphicsContext, usageType, totalBufferSize, pDataPtr, result);

		if (result != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		_insertBuffer(pNewConstantBuffer);

		return TOkValue<IConstantBuffer*>(pNewConstantBuffer);
	}

	TResult<IVertexDeclaration*> COGLGraphicsObjectManager::CreateVertexDeclaration()
	{
		E_RESULT_CODE result = RC_OK;

		IVertexDeclaration* pNewVertDecl = CreateOGLVertexDeclaration(result);

		if (result != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		_insertVertexDeclaration(pNewVertDecl);

		return TOkValue<IVertexDeclaration*>(pNewVertDecl);
	}

	TResult<TTextureSamplerId> COGLGraphicsObjectManager::CreateTextureSampler(const TTextureSamplerDesc& samplerDesc)
	{
		GLuint samplerHandler = 0x0;

		glGenSamplers(1, &samplerHandler);

		if (glGetError() != GL_NO_ERROR)
		{
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}
		
		glSamplerParameteri(samplerHandler, GL_TEXTURE_WRAP_S, COGLMappings::GetTextureAddressMode(samplerDesc.mUAddressMode));
		glSamplerParameteri(samplerHandler, GL_TEXTURE_WRAP_T, COGLMappings::GetTextureAddressMode(samplerDesc.mVAddressMode));
		glSamplerParameteri(samplerHandler, GL_TEXTURE_WRAP_R, COGLMappings::GetTextureAddressMode(samplerDesc.mWAddressMode));
		glSamplerParameteri(samplerHandler, GL_TEXTURE_MAG_FILTER, COGLMappings::GetMagFilterType(samplerDesc.mFilterFlags));
		glSamplerParameteri(samplerHandler, GL_TEXTURE_MIN_FILTER, COGLMappings::GetMinFilterType(samplerDesc.mFilterFlags));

		U32 samplerId = mTextureSamplersArray.size();

		mTextureSamplersArray.push_back(samplerHandler);

		return TOkValue<TTextureSamplerId>(samplerId);
	}

	TResult<GLuint> COGLGraphicsObjectManager::GetTextureSampler(TTextureSamplerId texSamplerId) const
	{
		if (texSamplerId <= mTextureSamplersArray.size())
		{
			return TErrorValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		return TOkValue<GLuint>(mTextureSamplersArray[texSamplerId]);
	}

	E_RESULT_CODE COGLGraphicsObjectManager::_freeTextureSamplers()
	{
		glDeleteSamplers(mTextureSamplersArray.size(), &mTextureSamplersArray[0]);
		
		if (glGetError() != GL_NO_ERROR)
		{
			return RC_FAIL;
		}

		mTextureSamplersArray.clear();

		return RC_OK;
	}


	IGraphicsObjectManager* CreateOGLGraphicsObjectManager(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		COGLGraphicsObjectManager* pManagerInstance = new (std::nothrow) COGLGraphicsObjectManager();

		if (!pManagerInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pManagerInstance->Init(pGraphicsContext);

		if (result != RC_OK)
		{
			delete pManagerInstance;

			pManagerInstance = nullptr;
		}

		return pManagerInstance;
	}
}