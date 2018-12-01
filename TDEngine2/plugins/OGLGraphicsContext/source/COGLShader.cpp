#include "./../include/COGLShader.h"
#include "./../include/COGLGraphicsContext.h"
#include "./../include/COGLShaderCompiler.h"
#include "./../include/COGLConstantBuffer.h"
#include <graphics/CBaseShader.h>


namespace TDEngine2
{
	COGLShader::COGLShader() :
		CBaseShader(), mShaderHandler(0)
	{
	}

	E_RESULT_CODE COGLShader::Reset()
	{
		mIsInitialized = false;

		if (mShaderHandler)
		{
			glDeleteShader(mShaderHandler);

			if (glGetError() != GL_NO_ERROR)
			{
				return RC_FAIL;
			}
		}

		E_RESULT_CODE result = RC_OK;

		if ((result = _freeUniformBuffers()) != RC_OK)
		{
			return result;
		}
				
		return RC_OK;
	}

	void COGLShader::Bind()
	{
		glUseProgram(mShaderHandler);
	}

	void COGLShader::Unbind()
	{
		glUseProgram(0);
	}

	E_RESULT_CODE COGLShader::_createInternalHandlers(const TShaderCompilerOutput* pCompilerData)
	{
		const TOGLShaderCompilerOutput* pOGLShaderCompilerData = dynamic_cast<const TOGLShaderCompilerOutput*>(pCompilerData);
		
		mShaderHandler = glCreateProgram();

		if (pOGLShaderCompilerData->mVertexShaderHandler)
		{
			glAttachShader(mShaderHandler, pOGLShaderCompilerData->mVertexShaderHandler);

			if (glGetError() != GL_NO_ERROR)
			{
				return RC_FAIL;
			}
		}

		if (pOGLShaderCompilerData->mFragmentShaderHandler)
		{
			glAttachShader(mShaderHandler, pOGLShaderCompilerData->mFragmentShaderHandler);

			if (glGetError() != GL_NO_ERROR)
			{
				return RC_FAIL;
			}
		}

		if (pOGLShaderCompilerData->mGeometryShaderHandler)
		{
			glAttachShader(mShaderHandler, pOGLShaderCompilerData->mGeometryShaderHandler);

			if (glGetError() != GL_NO_ERROR)
			{
				return RC_FAIL;
			}
		}

		glLinkProgram(mShaderHandler);

		if (glGetError() != GL_NO_ERROR)
		{
			return RC_FAIL;
		}

		I32 isLinked = 0;
		
		glGetProgramiv(mShaderHandler, GL_LINK_STATUS, &isLinked);

		if (glGetError() != GL_NO_ERROR)
		{
			return RC_FAIL;
		}

		if (!isLinked)
		{
			/// \todo reimplement error handling in other way
			GLint messageLength = 0;

			glGetProgramiv(mShaderHandler, GL_INFO_LOG_LENGTH, &messageLength);

			if (glGetError() != GL_NO_ERROR)
			{
				return RC_FAIL;
			}

			std::vector<GLchar> tmpErrorMsgBuffer(messageLength);

			/// retrieve error message
			glGetProgramInfoLog(mShaderHandler, messageLength, &messageLength, &tmpErrorMsgBuffer[0]);

			std::string errorMessageStr(tmpErrorMsgBuffer.begin(), tmpErrorMsgBuffer.end());

			glDeleteProgram(mShaderHandler);

			if (pOGLShaderCompilerData->mVertexShaderHandler)
			{
				glDeleteShader(pOGLShaderCompilerData->mVertexShaderHandler);

				if (glGetError() != GL_NO_ERROR)
				{
					return RC_FAIL;
				}
			}

			if (pOGLShaderCompilerData->mFragmentShaderHandler)
			{
				glDeleteShader(pOGLShaderCompilerData->mFragmentShaderHandler);

				if (glGetError() != GL_NO_ERROR)
				{
					return RC_FAIL;
				}
			}

			if (pOGLShaderCompilerData->mGeometryShaderHandler)
			{
				glDeleteShader(pOGLShaderCompilerData->mGeometryShaderHandler);

				if (glGetError() != GL_NO_ERROR)
				{
					return RC_FAIL;
				}
			}
		}

		if (pOGLShaderCompilerData->mVertexShaderHandler)
		{
			glDetachShader(mShaderHandler, pOGLShaderCompilerData->mVertexShaderHandler);

			if (glGetError() != GL_NO_ERROR)
			{
				return RC_FAIL;
			}
		}

		if (pOGLShaderCompilerData->mFragmentShaderHandler)
		{
			glDetachShader(mShaderHandler, pOGLShaderCompilerData->mFragmentShaderHandler);

			if (glGetError() != GL_NO_ERROR)
			{
				return RC_FAIL;
			}
		}

		if (pOGLShaderCompilerData->mGeometryShaderHandler)
		{
			glDetachShader(mShaderHandler, pOGLShaderCompilerData->mGeometryShaderHandler);

			if (glGetError() != GL_NO_ERROR)
			{
				return RC_FAIL;
			}
		}

		return _createUniformBuffers(pCompilerData);
	}

	E_RESULT_CODE COGLShader::_createUniformBuffers(const TShaderCompilerOutput* pCompilerData)
	{
		auto uniformBuffersInfo = pCompilerData->mUniformBuffersInfo;

		TUniformBufferDesc currDesc;

		E_RESULT_CODE result = RC_OK;

		mUniformBuffers.resize(uniformBuffersInfo.size());

		for (auto iter = uniformBuffersInfo.cbegin(); iter != uniformBuffersInfo.cend(); ++iter)
		{
			currDesc = (*iter).second;

			mUniformBuffers[currDesc.mSlot] = CreateOGLConstantBuffer(mpGraphicsContext, BUT_DYNAMIC, currDesc.mSize, nullptr, result);
		}

		return RC_OK;
	}

	void COGLShader::_bindUniformBuffer(U32 slot, IConstantBuffer* pBuffer)
	{
		pBuffer->Bind(slot);

		glUniformBlockBinding(mShaderHandler, slot, slot);
	}


	TDE2_API IShader* CreateOGLShader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
		TResourceId id, E_RESULT_CODE& result)
	{
		COGLShader* pShaderInstance = new (std::nothrow) COGLShader();

		if (!pShaderInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pShaderInstance->Init(pResourceManager, pGraphicsContext, name, id);

		if (result != RC_OK)
		{
			delete pShaderInstance;

			pShaderInstance = nullptr;
		}

		return pShaderInstance;
	}


	COGLShaderFactory::COGLShaderFactory() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE COGLShaderFactory::Init(IGraphicsContext* pGraphicsContext)
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

	E_RESULT_CODE COGLShaderFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IResource* COGLShaderFactory::Create(const TBaseResourceParameters* pParams) const
	{
		return nullptr;
	}

	IResource* COGLShaderFactory::CreateDefault(const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return dynamic_cast<IResource*>(CreateOGLShader(params.mpResourceManager, mpGraphicsContext, params.mName, params.mId, result));
	}

	U32 COGLShaderFactory::GetResourceTypeId() const
	{
		return CBaseShader::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateOGLShaderFactory(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		COGLShaderFactory* pShaderFactoryInstance = new (std::nothrow) COGLShaderFactory();

		if (!pShaderFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pShaderFactoryInstance->Init(pGraphicsContext);

		if (result != RC_OK)
		{
			delete pShaderFactoryInstance;

			pShaderFactoryInstance = nullptr;
		}

		return pShaderFactoryInstance;
	}
}