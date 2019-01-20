#include "./../include/COGLShader.h"
#include "./../include/COGLGraphicsContext.h"
#include "./../include/COGLShaderCompiler.h"
#include "./../include/COGLConstantBuffer.h"
#include "./../include/COGLUtils.h"
#include <graphics/ITexture.h>
#include <graphics/CBaseShader.h>
#include <cassert>


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
			GL_SAFE_CALL(glDeleteShader(mShaderHandler));
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
		CBaseShader::Bind();

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
			GL_SAFE_CALL(glAttachShader(mShaderHandler, pOGLShaderCompilerData->mVertexShaderHandler));
		}

		if (pOGLShaderCompilerData->mFragmentShaderHandler)
		{
			GL_SAFE_CALL(glAttachShader(mShaderHandler, pOGLShaderCompilerData->mFragmentShaderHandler));
		}

		if (pOGLShaderCompilerData->mGeometryShaderHandler)
		{
			GL_SAFE_CALL(glAttachShader(mShaderHandler, pOGLShaderCompilerData->mGeometryShaderHandler));
		}

		GL_SAFE_CALL(glLinkProgram(mShaderHandler));

		I32 isLinked = 0;
		
		GL_SAFE_CALL(glGetProgramiv(mShaderHandler, GL_LINK_STATUS, &isLinked))

		if (!isLinked)
		{
			/// \todo reimplement error handling in other way
			GLint messageLength = 0;

			GL_SAFE_CALL(glGetProgramiv(mShaderHandler, GL_INFO_LOG_LENGTH, &messageLength));

			std::vector<GLchar> tmpErrorMsgBuffer(messageLength);

			/// retrieve error message
			glGetProgramInfoLog(mShaderHandler, messageLength, &messageLength, &tmpErrorMsgBuffer[0]);

			std::string errorMessageStr(tmpErrorMsgBuffer.begin(), tmpErrorMsgBuffer.end());

			GL_SAFE_CALL(glDeleteProgram(mShaderHandler));

			if (pOGLShaderCompilerData->mVertexShaderHandler)
			{
				GL_SAFE_CALL(glDeleteShader(pOGLShaderCompilerData->mVertexShaderHandler));
			}

			if (pOGLShaderCompilerData->mFragmentShaderHandler)
			{
				GL_SAFE_CALL(glDeleteShader(pOGLShaderCompilerData->mFragmentShaderHandler));
			}

			if (pOGLShaderCompilerData->mGeometryShaderHandler)
			{
				GL_SAFE_CALL(glDeleteShader(pOGLShaderCompilerData->mGeometryShaderHandler));
			}
		}

		if (pOGLShaderCompilerData->mVertexShaderHandler)
		{
			GL_SAFE_CALL(glDetachShader(mShaderHandler, pOGLShaderCompilerData->mVertexShaderHandler));
		}

		if (pOGLShaderCompilerData->mFragmentShaderHandler)
		{
			GL_SAFE_CALL(glDetachShader(mShaderHandler, pOGLShaderCompilerData->mFragmentShaderHandler));
		}

		if (pOGLShaderCompilerData->mGeometryShaderHandler)
		{
			GL_SAFE_CALL(glDetachShader(mShaderHandler, pOGLShaderCompilerData->mGeometryShaderHandler));
		}
		
		return _createUniformBuffers(pCompilerData);
	}

	E_RESULT_CODE COGLShader::_createUniformBuffers(const TShaderCompilerOutput* pCompilerData)
	{
		auto uniformBuffersInfo = pCompilerData->mUniformBuffersInfo;

		TUniformBufferDesc currDesc;

		E_RESULT_CODE result = RC_OK;

		mUniformBuffers.resize(uniformBuffersInfo.size() - TotalNumberOfInternalConstantBuffers);

		/*!
			In nutshell, we store mappings from (engine slot -> internal GLSL buffer id) within mUniformBuffersMap for both internal and user-defined uniform buffers.
			But only user's buffers are processed by the shader itself, therefore we create only them and store into mUniformBuffers
		*/

		for (auto iter = uniformBuffersInfo.cbegin(); iter != uniformBuffersInfo.cend(); ++iter)
		{
			currDesc = (*iter).second;

			mUniformBuffersMap[currDesc.mSlot] = glGetUniformBlockIndex(mShaderHandler, (*iter).first.c_str());

			/// skip internal buffer's creation, because they are created separately by IGlobalShaderProperties implementation
			if ((currDesc.mFlags & E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL) == E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL)
			{
				/// bind this shader to internal uniform buffers here, so it can access right here

				GL_SAFE_CALL(glUniformBlockBinding(mShaderHandler, mUniformBuffersMap[currDesc.mSlot], currDesc.mSlot));

				continue;
			}
			
			assert((currDesc.mSlot - TotalNumberOfInternalConstantBuffers) >= 0);

			/// the offset is used because the shaders doesn't store internal buffer by themselves
			mUniformBuffers[currDesc.mSlot - TotalNumberOfInternalConstantBuffers] = CreateOGLConstantBuffer(mpGraphicsContext, BUT_DYNAMIC, currDesc.mSize, nullptr, result);
		}
		
		return RC_OK;
	}

	void COGLShader::_bindUniformBuffer(U32 slot, IConstantBuffer* pBuffer)
	{
		pBuffer->Bind(slot);

		glUniformBlockBinding(mShaderHandler, mUniformBuffersMap[slot], slot);
	}
	
	E_RESULT_CODE COGLShader::_createTexturesHashTable(const TShaderCompilerOutput* pCompilerData)
	{
		GL_SAFE_CALL(glUseProgram(mShaderHandler));

		auto shaderResourcesMap = pCompilerData->mShaderResourcesInfo;

		if (shaderResourcesMap.empty())
		{
			return RC_OK;
		}

		I16 currSlotIndex = 0;

		const C8* currName;

		for (auto currShaderResourceInfo : shaderResourcesMap)
		{
			currName = currShaderResourceInfo.first.c_str();

			currSlotIndex = glGetUniformLocation(mShaderHandler, currName);

			if (currSlotIndex < 0)
			{
				continue;
			}

			mTexturesHashTable[currName] = currSlotIndex;

			GL_SAFE_CALL(glUniform1i(currSlotIndex, currShaderResourceInfo.second.mSlot));

			mpTextures.resize(currSlotIndex + 1);

			mpTextures[currSlotIndex] = nullptr;
		}

		glUseProgram(0);

		return RC_OK;
	}


	TDE2_API IShader* CreateOGLShader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		COGLShader* pShaderInstance = new (std::nothrow) COGLShader();

		if (!pShaderInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pShaderInstance->Init(pResourceManager, pGraphicsContext, name);

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

	E_RESULT_CODE COGLShaderFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
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

	IResource* COGLShaderFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		return nullptr;
	}

	IResource* COGLShaderFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return dynamic_cast<IResource*>(CreateOGLShader(mpResourceManager, mpGraphicsContext, name, result));
	}

	U32 COGLShaderFactory::GetResourceTypeId() const
	{
		return CBaseShader::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateOGLShaderFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		COGLShaderFactory* pShaderFactoryInstance = new (std::nothrow) COGLShaderFactory();

		if (!pShaderFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pShaderFactoryInstance->Init(pResourceManager, pGraphicsContext);

		if (result != RC_OK)
		{
			delete pShaderFactoryInstance;

			pShaderFactoryInstance = nullptr;
		}

		return pShaderFactoryInstance;
	}
}