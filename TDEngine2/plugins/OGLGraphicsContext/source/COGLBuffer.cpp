#include "./../include/COGLBuffer.h"
#include "./../include/COGLMappings.h"
#include <core/IGraphicsContext.h>
#include <memory>


namespace TDEngine2
{
	COGLBuffer::COGLBuffer() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE COGLBuffer::Init(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, E_BUFFER_TYPE bufferType, U32 totalBufferSize, void* pDataPtr)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mBufferSize = totalBufferSize;

		mBufferUsageType = usageType;

		mBufferType = bufferType;

		glGenBuffers(1, &mBufferHandler);

		GLenum errorCode = 0;

		if ((errorCode = glGetError()) != GL_NO_ERROR)
		{
			return RC_FAIL;	/// \todo implement mapping from OGL errors to engine's ones
		}

		GLenum glInternalBufferType = _getBufferType(bufferType);

		glBindBuffer(glInternalBufferType, mBufferHandler);

		if ((errorCode = glGetError()) != GL_NO_ERROR)
		{
			return RC_FAIL;	/// \todo implement mapping from OGL errors to engine's ones
		}

		glBufferData(glInternalBufferType, totalBufferSize, pDataPtr, COGLMappings::GetUsageType(usageType));

		if ((errorCode = glGetError()) != GL_NO_ERROR)
		{
			return RC_FAIL;	/// \todo implement mapping from OGL errors to engine's ones
		}

		glBindBuffer(glInternalBufferType, 0);

		mBufferInternalData.mGLBuffer = mBufferHandler;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE COGLBuffer::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		glDeleteBuffers(1, &mBufferHandler);

		if (glGetError() != GL_NO_ERROR)
		{
			return RC_FAIL;	/// \todo implement mapping from OGL errors to engine's ones
		}

		delete this;

		mIsInitialized = false;

		return RC_OK;
	}

	E_RESULT_CODE COGLBuffer::Map(E_BUFFER_MAP_TYPE mapType)
	{
		glBindBuffer(_getBufferType(mBufferType), mBufferHandler);

		if (glGetError() != GL_NO_ERROR)
		{
			return RC_FAIL;	/// \todo implement mapping from OGL errors to engine's ones
		}

		mpMappedBufferData = glMapBuffer(_getBufferType(mBufferType), COGLMappings::GetBufferMapAccessType(mapType));

		if (glGetError() != GL_NO_ERROR)
		{
			return RC_FAIL;	/// \todo implement mapping from OGL errors to engine's ones
		}

		return RC_OK;
	}

	void COGLBuffer::Unmap()
	{
		glUnmapBuffer(_getBufferType(mBufferType));
	}

	E_RESULT_CODE COGLBuffer::Write(const void* pData, U32 size)
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

	const TBufferInternalData& COGLBuffer::GetInternalData() const
	{
		return mBufferInternalData;
	}

	U32 COGLBuffer::GetSize() const
	{
		return mBufferSize;
	}

	TDE2_API GLenum COGLBuffer::_getBufferType(E_BUFFER_TYPE type) const
	{
		switch (type)
		{
			case BT_VERTEX_BUFFER:
				return GL_ARRAY_BUFFER;
			case BT_INDEX_BUFFER:
				return GL_ELEMENT_ARRAY_BUFFER;
		}

		return 0;
	}


	TDE2_API IBuffer* CreateOGLBuffer(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, COGLBuffer::E_BUFFER_TYPE bufferType,
		U32 totalBufferSize, void* pDataPtr, E_RESULT_CODE& result)
	{
		COGLBuffer* pBufferInstance = new (std::nothrow) COGLBuffer();

		if (!pBufferInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pBufferInstance->Init(pGraphicsContext, usageType, bufferType, totalBufferSize, pDataPtr);

		if (result != RC_OK)
		{
			delete pBufferInstance;

			pBufferInstance = nullptr;
		}

		return dynamic_cast<IBuffer*>(pBufferInstance);
	}
}