#include "../include/COGLResources.h"
#include "../include/COGLMappings.h"
#include "../include/COGLUtils.h"
#include "../include/COGLGraphicsObjectManager.h"
#include <core/IGraphicsContext.h>
#include <graphics/CBaseGraphicsPipeline.h>
#include <memory>
#include <cstring>


namespace TDEngine2
{
	static GLenum GetBufferType(E_BUFFER_TYPE type)
	{
		switch (type)
		{
			case E_BUFFER_TYPE::VERTEX:
				return GL_ARRAY_BUFFER;
			case E_BUFFER_TYPE::INDEX:
				return GL_ELEMENT_ARRAY_BUFFER;
			case E_BUFFER_TYPE::CONSTANT:
				return GL_UNIFORM_BUFFER;
			case E_BUFFER_TYPE::STRUCTURED:
				return GL_SHADER_STORAGE_BUFFER;
		}

		return 0;
	}


	static TResult<GLuint> CreateBufferInternal(E_BUFFER_TYPE type, E_BUFFER_USAGE_TYPE usageType, USIZE size, const void* pData)
	{
		GLuint bufferHandle = 0;

		GL_SAFE_TRESULT_CALL(glGenBuffers(1, &bufferHandle));

		const GLenum glInternalBufferType = GetBufferType(type);

		GL_SAFE_TRESULT_CALL(glBindBuffer(glInternalBufferType, bufferHandle));

#if TDE2_DEBUG_MODE
		if (!pData)
		{
			std::vector<U8> zeroInitializedSpace(size);
			GL_SAFE_TRESULT_CALL(glBufferData(glInternalBufferType, size, zeroInitializedSpace.data(), COGLMappings::GetUsageType(usageType)));
		}
		else
		{
			GL_SAFE_TRESULT_CALL(glBufferData(glInternalBufferType, size, pData, COGLMappings::GetUsageType(usageType)));
		}
#else
		GL_SAFE_TRESULT_CALL(glBufferData(glInternalBufferType, size, pData, COGLMappings::GetUsageType(usageType)));
#endif
		GL_SAFE_TRESULT_CALL(glBindBuffer(glInternalBufferType, 0));

		return Wrench::TOkValue<GLuint>(bufferHandle);
	}


	COGLBuffer::COGLBuffer() :
		CBaseObject()
	{
	}

	E_RESULT_CODE COGLBuffer::Init(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mBufferSize = params.mTotalBufferSize;
		mBufferUsageType = params.mUsageType;
		mBufferType = params.mBufferType;

		auto createBufferHandleResult = CreateBufferInternal(mBufferType, mBufferUsageType, mBufferSize, params.mpDataPtr);
		if (createBufferHandleResult.HasError())
		{
			return createBufferHandleResult.GetError();
		}

		mBufferHandler = createBufferHandleResult.Get();

		mInitParams = params;

#if TDE2_DEBUG_MODE
		if (GLEW_KHR_debug)
		{
			glObjectLabel(GL_BUFFER, mBufferHandler, -1, mInitParams.mName);
		}
#endif

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE COGLBuffer::_onFreeInternal()
	{
		GL_SAFE_CALL(glDeleteBuffers(1, &mBufferHandler));
		return RC_OK;
	}

	E_RESULT_CODE COGLBuffer::Map(E_BUFFER_MAP_TYPE mapType, USIZE offset)
	{
		/// \todo GL_SAFE_CALL wrapper causes GL_INVALID_VALUE is raised by RenderDoc
		/// but everything works well in standalone mode and within MSVC
		glBindBuffer(GetBufferType(mBufferType), mBufferHandler);

		mpMappedBufferData = reinterpret_cast<void*>(reinterpret_cast<U8*>(glMapBuffer(GetBufferType(mBufferType), COGLMappings::GetBufferMapAccessType(mapType))) + offset);
		
#if TDE2_DEBUG_MODE
		++mLockChecker;
#endif

		return COGLMappings::GetErrorCode(glGetError());
	}

	void COGLBuffer::Unmap()
	{
#if TDE2_DEBUG_MODE
		--mLockChecker;
#endif

		GL_SAFE_VOID_CALL(glUnmapBuffer(GetBufferType(mBufferType)));
	}

	E_RESULT_CODE COGLBuffer::Write(const void* pData, USIZE size)
	{
		if (!mpMappedBufferData || size > mBufferSize)
		{
			return RC_FAIL;
		}

		memcpy(mpMappedBufferData, pData, size);

		return RC_OK;
	}

	void* COGLBuffer::Read()
	{
		return mpMappedBufferData;
	}

	E_RESULT_CODE COGLBuffer::Resize(USIZE newSize)
	{
		auto createBufferHandleResult = CreateBufferInternal(mBufferType, mBufferUsageType, newSize, nullptr);
		if (createBufferHandleResult.HasError())
		{
			return createBufferHandleResult.GetError();
		}

		if (mpMappedBufferData)
		{
			Unmap();
		}

		GL_SAFE_CALL(glDeleteBuffers(1, &mBufferHandler));

		mBufferHandler = createBufferHandleResult.Get();

		mBufferSize = newSize;
		mInitParams.mTotalBufferSize = mBufferSize;

		return RC_OK;
	}

	void* COGLBuffer::GetInternalData()
	{
		return reinterpret_cast<void*>(&mBufferHandler);
	}

	USIZE COGLBuffer::GetSize() const
	{
		return mBufferSize;
	}

	const TInitBufferParams& COGLBuffer::GetParams() const
	{
		return mInitParams;
	}

	GLuint COGLBuffer::GetOGLHandle()
	{
		return mBufferHandler;
	}


	IBuffer* CreateOGLBuffer(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IBuffer, COGLBuffer, result, pGraphicsContext, params);
	}


	/*!
		\brief COGLShader's definition
	*/

	static const std::string ShaderLanguageId = "glsl";


	COGLShader::COGLShader() :
		CBaseShader(), mShaderHandler(0)
	{
	}

	E_RESULT_CODE COGLShader::Reset()
	{
		mIsInitialized = false;

		if (mShaderHandler)
		{
			GL_SAFE_CALL(glDeleteProgram(mShaderHandler));
		}

		return RC_OK;
	}

	void COGLShader::Bind()
	{
		CBaseShader::Bind();

		GL_SAFE_VOID_CALL(glUseProgram(mShaderHandler));
	}

	void COGLShader::Unbind()
	{
		CBaseShader::Unbind();
		GL_SAFE_VOID_CALL(glUseProgram(0));
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

		if (pOGLShaderCompilerData->mComputeShaderHandler)
		{
			GL_SAFE_CALL(glAttachShader(mShaderHandler, pOGLShaderCompilerData->mComputeShaderHandler));
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
				LOG_ERROR(Wrench::StringUtils::Format("[OGLShader] Some error happened during compilation: \n{0}", errorMessageStr));

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

				if (pOGLShaderCompilerData->mComputeShaderHandler)
				{
					GL_SAFE_CALL(glDeleteShader(pOGLShaderCompilerData->mComputeShaderHandler));
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

		if (pOGLShaderCompilerData->mComputeShaderHandler)
		{
			GL_SAFE_CALL(glDetachShader(mShaderHandler, pOGLShaderCompilerData->mComputeShaderHandler));
		}

		return _createUniformBuffers(pCompilerData);
	}

	E_RESULT_CODE COGLShader::_createUniformBuffers(const TShaderCompilerOutput* pCompilerData)
	{
		auto uniformBuffersInfo = pCompilerData->mUniformBuffersInfo;

		E_RESULT_CODE result = RC_OK;

		mUniformBuffers.resize(uniformBuffersInfo.size() - TotalNumberOfInternalConstantBuffers);

		auto pGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();

		/*!
			In nutshell, we store mappings from (engine slot -> internal GLSL buffer id) within mUniformBuffersMap for both internal and user-defined uniform buffers.
			But only user's buffers are processed by the shader itself, therefore we create only them and store into mUniformBuffers
		*/

		for (auto iter = uniformBuffersInfo.begin(); iter != uniformBuffersInfo.end(); ++iter)
		{
			TUniformBufferDesc& currDesc = (*iter).second;

			mUniformBuffersMap[currDesc.mSlot] = glGetUniformBlockIndex(mShaderHandler, (*iter).first.c_str());

			/// skip internal buffer's creation, because they are created separately by IGlobalShaderProperties implementation
			if ((currDesc.mFlags & E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL) == E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL)
			{
				if (GL_INVALID_INDEX == mUniformBuffersMap[currDesc.mSlot])
				{
					continue; // \note means the UBO isn't used
				}

				/// bind this shader to internal uniform buffers here, so it can access right here

				GL_SAFE_CALL(glUniformBlockBinding(mShaderHandler, mUniformBuffersMap[currDesc.mSlot], currDesc.mSlot));

				continue;
			}

			TDE2_ASSERT((currDesc.mSlot - TotalNumberOfInternalConstantBuffers) >= 0);

			/// \note Ensure that we compute correct size of the block
			I32 reflectionBlockSize = 0;
			glGetActiveUniformBlockiv(mShaderHandler, mUniformBuffersMap[currDesc.mSlot], GL_UNIFORM_BLOCK_DATA_SIZE, &reflectionBlockSize);

			if (currDesc.mSize != reflectionBlockSize)
			{
				auto& uniformBlockInfo = mpShaderMeta->mUniformBuffersInfo[iter->first];
				uniformBlockInfo.mSize = static_cast<U32>(reflectionBlockSize);

				currDesc.mSize = uniformBlockInfo.mSize;

				/// \note Recompute sizes of individual members of uniform buffers
				I32 numOfActiveUniforms = 0;
				glGetActiveUniformBlockiv(mShaderHandler, mUniformBuffersMap[currDesc.mSlot], GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numOfActiveUniforms);

				std::vector<I32> activeUniformsIndices;
				activeUniformsIndices.resize(static_cast<size_t>(numOfActiveUniforms));

				glGetActiveUniformBlockiv(mShaderHandler, mUniformBuffersMap[currDesc.mSlot], GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, &activeUniformsIndices.front());

				I32 maxUniformNameLength = 0;
				glGetProgramiv(mShaderHandler, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformNameLength);

				std::string uniformName;
				uniformName.resize(maxUniformNameLength);

				I32 currUniformNameLength = 0;
				I32 currUniformSize = 0;

				GLenum type;

				auto& uniformBlockVariables = uniformBlockInfo.mVariables;

				for (const I32& currUniformIndex : activeUniformsIndices)
				{
					glGetActiveUniform(mShaderHandler, currUniformIndex, maxUniformNameLength, &currUniformNameLength, &currUniformSize, &type, &uniformName[0]);

					auto it = std::find_if(uniformBlockVariables.begin(), uniformBlockVariables.end(), [uniformName](auto&& entity)
						{
							return Wrench::StringUtils::StartsWith(uniformName, entity.mName);
						});

					if (it != uniformBlockVariables.end())
					{
						it->mSize = static_cast<U32>(COGLMappings::GetTypeSize(type) * currUniformSize);
					}
				}
			}

			/// the offset is used because the shaders doesn't store internal buffer by themselves
			mUniformBuffers[currDesc.mSlot - TotalNumberOfInternalConstantBuffers] =
				pGraphicsObjectManager->CreateBuffer({ E_BUFFER_USAGE_TYPE::DYNAMIC, E_BUFFER_TYPE::CONSTANT, currDesc.mSize, nullptr }).Get();
		}

		return RC_OK;
	}

#if 0
	static TResult<GLuint> TryToCreatePrecompiledShader(IShaderCache* pShaderCache, const TShaderParameters* pShaderParams, E_SHADER_STAGE_TYPE stageType)
	{
		TDE2_PROFILER_SCOPE("TryToCreatePrecompiledShader");

		GLuint shaderHandler = glCreateShader(COGLMappings::GetShaderStageType(stageType));
		if (glGetError() != GL_NO_ERROR)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(COGLMappings::GetErrorCode(glGetError()));
		}

		auto it = pShaderParams->mStages.find(stageType);
		if (it != pShaderParams->mStages.cend())
		{
			if (it->second.mBytecodeInfo.find(ShaderLanguageId) == it->second.mBytecodeInfo.end())
			{
				return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
			}

			auto&& bytecode = std::move(pShaderCache->GetBytecode(it->second.mBytecodeInfo.at(ShaderLanguageId)));

			glShaderBinary(1, &shaderHandler, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, bytecode.data(), static_cast<GLsizei>(bytecode.size()));
			if (glGetError() != GL_NO_ERROR)
			{
				return Wrench::TErrValue<E_RESULT_CODE>(COGLMappings::GetErrorCode(glGetError()));
			}

			glSpecializeShader(shaderHandler, it->second.mEntrypoint.c_str(), 0, 0, 0);
			if (glGetError() != GL_NO_ERROR)
			{
				return Wrench::TErrValue<E_RESULT_CODE>(COGLMappings::GetErrorCode(glGetError()));
			}

			return Wrench::TOkValue<GLuint>(shaderHandler);
		}

		return Wrench::TOkValue<GLuint>(0);
	}
#endif

	void COGLShader::_bindUniformBuffer(U32 slot, TBufferHandleId uniformsBufferHandle)
	{
		CBaseShader::_bindUniformBuffer(slot, uniformsBufferHandle);
		GL_SAFE_VOID_CALL(glUniformBlockBinding(mShaderHandler, mUniformBuffersMap[slot], slot));
	}

	E_RESULT_CODE COGLShader::_createTexturesHashTable(const TShaderCompilerOutput* pCompilerData)
	{
		E_RESULT_CODE result = CBaseShader::_createTexturesHashTable(pCompilerData);

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

			auto& desc = currShaderResourceInfo.second;

			if (E_SHADER_RESOURCE_TYPE::SRT_SAMPLER_STATE == desc.mType
				|| E_SHADER_RESOURCE_TYPE::SRT_RW_STRUCTURED_BUFFER == desc.mType
				|| E_SHADER_RESOURCE_TYPE::SRT_STRUCTURED_BUFFER == desc.mType)
			{
				continue;
			}

			currSlotIndex = glGetUniformLocation(mShaderHandler, currName);
			if (currSlotIndex < 0)
			{
				continue;
			}

			GL_SAFE_CALL(glUniform1i(currSlotIndex, desc.mSlot));
		}

		glUseProgram(0);

		return result;
	}


	IShader* CreateOGLShader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IShader, COGLShader, result, pResourceManager, pGraphicsContext, name);
	}


	/*!
		class COGLShaderFactory

		\brief The class is an abstract factory of COGLShader objects that
		is used by a resource manager
	*/

	class COGLShaderFactory : public CBaseObject, public IShaderFactory
	{
		public:
			friend IResourceFactory* CreateOGLShaderFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a shader factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext) override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			IResource* Create(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			IResource* CreateDefault(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the factory serves

				\return The method returns an identifier of a resource's type, which
				the factory serves
			*/

			TypeId GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(COGLShaderFactory)
		protected:
			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};


	COGLShaderFactory::COGLShaderFactory() :
		CBaseObject()
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

	IResource* COGLShaderFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		return nullptr;
	}

	IResource* COGLShaderFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return dynamic_cast<IResource*>(CreateOGLShader(mpResourceManager, mpGraphicsContext, name, result));
	}

	TypeId COGLShaderFactory::GetResourceTypeId() const
	{
		return IShader::GetTypeId();
	}


	IResourceFactory* CreateOGLShaderFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, COGLShaderFactory, result, pResourceManager, pGraphicsContext);
	}


	/*!
		\brief COGLTextureImpl's definition
	*/

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
			glObjectLabel(GL_TEXTURE, textureHandle, -1, params.mName);
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


	ITextureImpl* CreateOGLTextureImpl(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ITextureImpl, COGLTextureImpl, result, pGraphicsContext, params);
	}


	/*!
		\brief COGLVertexDeclaration's definition
	*/

	static GLuint DefaultVAOHandler = 0;
	static GLuint DefaultVBOHandler = 0;


	COGLVertexDeclaration::COGLVertexDeclaration() :
		CVertexDeclaration()
	{
	}


	static TResult<GLuint> DoesHandleExist(IGraphicsContext* pGraphicsContext, const COGLVertexDeclaration::TVAORegistryNode& registry, const CStaticArray<TBufferHandleId>& pVertexBuffersArray)
	{
		auto pGraphicsObjectManager = pGraphicsContext->GetGraphicsObjectManager();

		const COGLVertexDeclaration::TVAORegistryNode* pCurrNode = &registry;

		U32 internalBufferHandle = 0x0;

		for (U32 i = 0; i < pVertexBuffersArray.GetSize(); ++i)
		{
			internalBufferHandle = TBufferHandleId::Invalid == pVertexBuffersArray[i] ? DefaultVBOHandler : *reinterpret_cast<GLuint*>(pGraphicsObjectManager->GetBufferPtr(pVertexBuffersArray[i])->GetInternalData());

			if (pCurrNode->mChildren.find(internalBufferHandle) == pCurrNode->mChildren.cend())
			{
				return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
			}

			pCurrNode = pCurrNode->mChildren.at(internalBufferHandle).get();
		}

		return Wrench::TOkValue<GLuint>(pCurrNode->mVAOHandle);
	}

	TResult<GLuint> COGLVertexDeclaration::GetVertexArrayObject(IGraphicsContext* pGraphicsContext, const CStaticArray<TBufferHandleId>& pVertexBuffersArray)
	{
		if (pVertexBuffersArray.IsEmpty())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		if (DefaultVBOHandler == 0)
		{
			GL_SAFE_VOID_CALL(glGenBuffers(1, &DefaultVBOHandler));
			GL_SAFE_VOID_CALL(glBindBuffer(GL_ARRAY_BUFFER, DefaultVBOHandler));
			GL_SAFE_VOID_CALL(glBufferData(GL_ARRAY_BUFFER, 1024, nullptr, GL_STATIC_DRAW));
			GL_SAFE_VOID_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
		}

		GLuint vaoHandler = 0x0;

		if (pVertexBuffersArray.GetSize() == 1 && TBufferHandleId::Invalid == pVertexBuffersArray[0])
		{
			if (DefaultVAOHandler == 0)
			{
				GL_SAFE_VOID_CALL(glGenVertexArrays(1, &DefaultVAOHandler));
			}

			GL_SAFE_VOID_CALL(glBindVertexArray(DefaultVAOHandler));

			return Wrench::TOkValue<GLuint>(DefaultVAOHandler);
		}

		auto doesExistResult = DoesHandleExist(pGraphicsContext, mRootNode, pVertexBuffersArray);
		if (doesExistResult.IsOk())
		{
			return doesExistResult;
		}

		GL_SAFE_VOID_CALL(glGenVertexArrays(1, &vaoHandler));
		GL_SAFE_VOID_CALL(glBindVertexArray(vaoHandler));

		if (glGetError() != GL_NO_ERROR)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		/// generate vertex attribute pointers
		GLuint currIndex = 0;

		E_FORMAT_TYPE currFormat = FT_UNKNOWN;

		U32 currInstanceDivisorIndex = (std::numeric_limits<U32>::max)();
		U32 nextInstanceDivisorIndex = currInstanceDivisorIndex;
		U32 currInstancesPerData = 0;
		U32 nextInstancesPerData = 0;
		USIZE currOffset = 0;
		U32 vertexStrideSize = GetStrideSize(0);

		TInstancingInfoArray::const_iterator instancingInfoIter = mInstancingInfo.cbegin();

		/// extract the information about next position where the new instancing block begins at
		if (!mInstancingInfo.empty())
		{
			std::tie(nextInstanceDivisorIndex, nextInstancesPerData) = *instancingInfoIter;
		}

		auto pGraphicsObjectManager = pGraphicsContext->GetGraphicsObjectManager();

		GLuint currBufferHandle = TBufferHandleId::Invalid == pVertexBuffersArray[0] ? DefaultVBOHandler : *reinterpret_cast<GLuint*>(pGraphicsObjectManager->GetBufferPtr(pVertexBuffersArray[0])->GetInternalData());

		GL_SAFE_VOID_CALL(glBindBuffer(GL_ARRAY_BUFFER, currBufferHandle)); /// bind the first VBO by default as a main vertex buffer

		TVAORegistryNode* pCurrNode = _insertNewNode(&mRootNode, currBufferHandle);

		for (auto iter = mElements.cbegin(); iter != mElements.cend(); ++iter, ++currIndex)
		{
			currFormat = (*iter).mFormatType;

			/// provide information about a per instance rate
			if (currIndex >= currInstanceDivisorIndex && currIndex <= nextInstanceDivisorIndex)
			{
				GL_SAFE_VOID_CALL(glVertexAttribDivisor(currIndex, currInstancesPerData));
			}
			else if (currIndex == nextInstanceDivisorIndex) /// reach the next instancing group
			{
				currOffset = 0; /// reset the current offset because of a new block begins

				/// bind the buffer that is related with the group
				currBufferHandle = *reinterpret_cast<GLuint*>(pGraphicsObjectManager->GetBufferPtr(pVertexBuffersArray[(*iter).mSource])->GetInternalData());
				GL_SAFE_VOID_CALL(glBindBuffer(GL_ARRAY_BUFFER, currBufferHandle));

				pCurrNode = _insertNewNode(pCurrNode, currBufferHandle);

				currInstanceDivisorIndex = nextInstanceDivisorIndex;
				currInstancesPerData = nextInstancesPerData;

				GL_SAFE_VOID_CALL(glVertexAttribDivisor(currIndex, currInstancesPerData));

				vertexStrideSize = GetStrideSize((*iter).mSource);

				if (instancingInfoIter + 1 != mInstancingInfo.cend())
				{
					std::tie(nextInstanceDivisorIndex, nextInstancesPerData) = *(instancingInfoIter++);
				}
				else
				{
					nextInstanceDivisorIndex = (std::numeric_limits<U32>::max)();
				}
			}

			GL_SAFE_VOID_CALL(glVertexAttribPointer(currIndex, COGLMappings::GetNumOfChannelsOfFormat(currFormat),
				COGLMappings::GetBaseTypeOfFormat(currFormat),
				COGLMappings::IsFormatNormalized(currFormat),
				vertexStrideSize, reinterpret_cast<void*>(currOffset)));

			currOffset += COGLMappings::GetFormatSize(currFormat);

			GL_SAFE_VOID_CALL(glEnableVertexAttribArray(currIndex));
		}

		pCurrNode->mVAOHandle = vaoHandler;

		GL_SAFE_VOID_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
		GL_SAFE_VOID_CALL(glBindVertexArray(0));

		return Wrench::TOkValue<GLuint>(vaoHandler);
	}

	void COGLVertexDeclaration::Bind(IGraphicsContext* pGraphicsContext, const CStaticArray<TBufferHandleId>& pVertexBuffersArray, IShader* pShader)
	{
		GL_SAFE_VOID_CALL(glBindVertexArray(GetVertexArrayObject(pGraphicsContext, pVertexBuffersArray).Get()));
	}

	E_RESULT_CODE COGLVertexDeclaration::_onFreeInternal()
	{
		if (DefaultVBOHandler != 0)
		{
			glDeleteBuffers(1, &DefaultVBOHandler);
		}

		if (DefaultVAOHandler != 0)
		{
			glDeleteBuffers(1, &DefaultVAOHandler);
		}

		return RC_OK;
	}

	COGLVertexDeclaration::TVAORegistryNode* COGLVertexDeclaration::_insertNewNode(TVAORegistryNode* pCurrNode, U32 handle)
	{
		if (pCurrNode->mChildren.find(handle) != pCurrNode->mChildren.cend())
		{
			return pCurrNode->mChildren[handle].get();
		}

		pCurrNode->mChildren[handle] = std::make_unique<TVAORegistryNode>();

		return pCurrNode->mChildren[handle].get();
	}


	IVertexDeclaration* CreateOGLVertexDeclaration(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IVertexDeclaration, COGLVertexDeclaration, result);
	}


	/*!
		\brief COGLGraphicsPipeline's definition
	*/

	class CGLGraphicsPipeline : public CBaseGraphicsPipeline
	{
		public:
			friend IGraphicsPipeline* CreateGLGraphicsPipeline(IGraphicsContext*, const TGraphicsPipelineConfigDesc&, E_RESULT_CODE&);
		public:
			E_RESULT_CODE Bind() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CGLGraphicsPipeline)
		private:
			COGLGraphicsObjectManager* mpGLGraphicsObjectManagerImpl = nullptr;

			TBlendStateId              mBlendStateHandle        = TBlendStateId::Invalid;
			TDepthStencilStateId       mDepthStencilStateHandle = TDepthStencilStateId::Invalid;
			TRasterizerStateId         mRasterizerStateHandle   = TRasterizerStateId::Invalid;
	};

	CGLGraphicsPipeline::CGLGraphicsPipeline() :
		CBaseGraphicsPipeline()
	{
	}

	E_RESULT_CODE CGLGraphicsPipeline::Bind()
	{
		if (!mpGraphicsObjectManager)
		{
			return RC_FAIL;
		}

		if (!mpGLGraphicsObjectManagerImpl)
		{
			mpGLGraphicsObjectManagerImpl = dynamic_cast<COGLGraphicsObjectManager*>(mpGraphicsObjectManager);
		}

		if (mBlendStateHandle == TBlendStateId::Invalid)
		{
			mBlendStateHandle = mpGLGraphicsObjectManagerImpl->CreateBlendState(mConfig.mBlendStateParams).Get();
		}

		mpGraphicsContext->BindBlendState(mBlendStateHandle);

		if (mDepthStencilStateHandle == TDepthStencilStateId::Invalid)
		{
			mDepthStencilStateHandle = mpGLGraphicsObjectManagerImpl->CreateDepthStencilState(mConfig.mDepthStencilStateParams).Get();
		}

		mpGraphicsContext->BindDepthStencilState(mDepthStencilStateHandle, mConfig.mDepthStencilStateParams.mStencilRefValue);

		if (mRasterizerStateHandle == TRasterizerStateId::Invalid)
		{
			mRasterizerStateHandle = mpGLGraphicsObjectManagerImpl->CreateRasterizerState(mConfig.mRasterizerStateParams).Get();
		}

		mpGraphicsContext->BindRasterizerState(mRasterizerStateHandle);

		return RC_OK;
	}


	IGraphicsPipeline* CreateGLGraphicsPipeline(IGraphicsContext* pGraphicsContext, const TGraphicsPipelineConfigDesc& config, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IGraphicsPipeline, CGLGraphicsPipeline, result, pGraphicsContext, config);
	}
}