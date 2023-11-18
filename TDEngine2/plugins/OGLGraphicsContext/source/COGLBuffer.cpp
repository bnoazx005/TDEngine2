#include "../include/COGLBuffer.h"
#include "../include/COGLMappings.h"
#include "../include/COGLUtils.h"
#include <core/IGraphicsContext.h>
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
		GL_SAFE_TRESULT_CALL(glBufferData(glInternalBufferType, size, pData, COGLMappings::GetUsageType(usageType)));
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


	TDE2_API IBuffer* CreateOGLBuffer(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IBuffer, COGLBuffer, result, pGraphicsContext, params);
	}
}