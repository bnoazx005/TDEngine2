#include "../include/COGLBuffer.h"
#include "../include/COGLMappings.h"
#include "../include/COGLUtils.h"
#include <core/IGraphicsContext.h>
#include <memory>
#include <cstring>


namespace TDEngine2
{
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
		mUsedBytesSize = 0;

		mBufferUsageType = params.mUsageType;

		mBufferType = params.mBufferType;

		GL_SAFE_CALL(glGenBuffers(1, &mBufferHandler));

		GLenum glInternalBufferType = _getBufferType(mBufferType);

		GL_SAFE_CALL(glBindBuffer(glInternalBufferType, mBufferHandler));
		GL_SAFE_CALL(glBufferData(glInternalBufferType, mBufferSize, params.mpDataPtr, COGLMappings::GetUsageType(mBufferUsageType)));
		GL_SAFE_CALL(glBindBuffer(glInternalBufferType, 0));

		mInitParams = params;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE COGLBuffer::_onFreeInternal()
	{
		GL_SAFE_CALL(glDeleteBuffers(1, &mBufferHandler));
		return RC_OK;
	}

	E_RESULT_CODE COGLBuffer::Map(E_BUFFER_MAP_TYPE mapType)
	{
		/// \todo GL_SAFE_CALL wrapper causes GL_INVALID_VALUE is raised by RenderDoc
		/// but everything works well in standalone mode and within MSVC
		glBindBuffer(_getBufferType(mBufferType), mBufferHandler);

		mpMappedBufferData = glMapBuffer(_getBufferType(mBufferType), COGLMappings::GetBufferMapAccessType(mapType));

#if TDE2_DEBUG_MODE
		++mLockChecker;
#endif

		if (glGetError() != GL_NO_ERROR)
		{
			return RC_FAIL;	/// \todo implement mapping from OGL errors to engine's ones
		}

		return RC_OK;
	}

	void COGLBuffer::Unmap()
	{
#if TDE2_DEBUG_MODE
		--mLockChecker;
#endif

		GL_SAFE_VOID_CALL(glUnmapBuffer(_getBufferType(mBufferType)));
	}

	E_RESULT_CODE COGLBuffer::Write(const void* pData, USIZE size)
	{
		if (!mpMappedBufferData || size > mBufferSize)
		{
			return RC_FAIL;
		}

		memcpy(mpMappedBufferData, pData, size);

		mUsedBytesSize += size;

		return RC_OK;
	}

	void* COGLBuffer::Read()
	{
		return mpMappedBufferData;
	}

	void* COGLBuffer::GetInternalData()
	{
		return reinterpret_cast<void*>(&mBufferHandler);
	}

	USIZE COGLBuffer::GetSize() const
	{
		return mBufferSize;
	}

	USIZE COGLBuffer::GetUsedSize() const
	{
		return mUsedBytesSize;
	}

	const TInitBufferParams& COGLBuffer::GetParams() const
	{
		return mInitParams;
	}

	TDE2_API GLenum COGLBuffer::_getBufferType(E_BUFFER_TYPE type) const
	{
		switch (type)
		{
			case E_BUFFER_TYPE::BT_VERTEX_BUFFER:
				return GL_ARRAY_BUFFER;
			case E_BUFFER_TYPE::BT_INDEX_BUFFER:
				return GL_ELEMENT_ARRAY_BUFFER;
			case E_BUFFER_TYPE::BT_CONSTANT_BUFFER:
				return GL_UNIFORM_BUFFER;
			case E_BUFFER_TYPE::BT_STRUCTURED_BUFFER:
				return GL_SHADER_STORAGE_BUFFER;
		}

		return 0;
	}


	TDE2_API IBuffer* CreateOGLBuffer(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IBuffer, COGLBuffer, result, pGraphicsContext, params);
	}
}