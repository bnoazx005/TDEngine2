#include "./../include/COGLBuffer.h"
#include "./../include/COGLMappings.h"
#include "./../include/COGLUtils.h"
#include <core/IGraphicsContext.h>
#include <memory>
#include <cstring>


namespace TDEngine2
{
	COGLBuffer::COGLBuffer() :
		CBaseObject()
	{
	}

	E_RESULT_CODE COGLBuffer::Init(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, E_BUFFER_TYPE bufferType, USIZE totalBufferSize, const void* pDataPtr)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mBufferSize = totalBufferSize;
		mUsedBytesSize = 0;

		mBufferUsageType = usageType;

		mBufferType = bufferType;

		GL_SAFE_CALL(glGenBuffers(1, &mBufferHandler));

		GLenum glInternalBufferType = _getBufferType(bufferType);

		GL_SAFE_CALL(glBindBuffer(glInternalBufferType, mBufferHandler));
		GL_SAFE_CALL(glBufferData(glInternalBufferType, totalBufferSize, pDataPtr, COGLMappings::GetUsageType(usageType)));
		GL_SAFE_CALL(glBindBuffer(glInternalBufferType, 0));

		mBufferInternalData.mGLBuffer = mBufferHandler;

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

	const TBufferInternalData& COGLBuffer::GetInternalData() const
	{
		return mBufferInternalData;
	}

	USIZE COGLBuffer::GetSize() const
	{
		return mBufferSize;
	}

	USIZE COGLBuffer::GetUsedSize() const
	{
		return mUsedBytesSize;
	}

	TDE2_API GLenum COGLBuffer::_getBufferType(E_BUFFER_TYPE type) const
	{
		switch (type)
		{
			case BT_VERTEX_BUFFER:
				return GL_ARRAY_BUFFER;
			case BT_INDEX_BUFFER:
				return GL_ELEMENT_ARRAY_BUFFER;
			case BT_CONSTANT_BUFFER:
				return GL_UNIFORM_BUFFER;
		}

		return 0;
	}


	TDE2_API IBuffer* CreateOGLBuffer(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, COGLBuffer::E_BUFFER_TYPE bufferType,
		USIZE totalBufferSize, const void* pDataPtr, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IBuffer, COGLBuffer, result, pGraphicsContext, usageType, bufferType, totalBufferSize, pDataPtr);
	}
}