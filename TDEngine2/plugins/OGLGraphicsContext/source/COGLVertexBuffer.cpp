#include "./../include/COGLVertexBuffer.h"
#include "./../include/COGLBuffer.h"
#include <core/IGraphicsContext.h>
#include <memory>


namespace TDEngine2
{
	COGLVertexBuffer::COGLVertexBuffer() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE COGLVertexBuffer::Init(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, U32 totalBufferSize, void* pDataPtr)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		mpBufferImpl = CreateOGLBuffer(pGraphicsContext, usageType, COGLBuffer::BT_VERTEX_BUFFER, totalBufferSize, pDataPtr, result);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE COGLVertexBuffer::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		if ((result = mpBufferImpl->Free()) != RC_OK)
		{
			return result;
		}

		delete this;

		mIsInitialized = false;

		return RC_OK;
	}

	E_RESULT_CODE COGLVertexBuffer::Map(E_BUFFER_MAP_TYPE mapType)
	{
		return mpBufferImpl->Map(mapType);
	}

	void COGLVertexBuffer::Unmap()
	{
		return mpBufferImpl->Unmap();
	}

	E_RESULT_CODE COGLVertexBuffer::Write(const void* pData, U32 size)
	{
		return mpBufferImpl->Write(pData, size);
	}

	void* COGLVertexBuffer::Read()
	{
		return mpBufferImpl->Read();
	}

	const TBufferInternalData& COGLVertexBuffer::GetInternalData() const
	{
		return mpBufferImpl->GetInternalData();
	}

	U32 COGLVertexBuffer::GetSize() const
	{
		return mpBufferImpl->GetSize();
	}


	TDE2_API IVertexBuffer* CreateOGLVertexBuffer(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType,
												  U32 totalBufferSize, void* pDataPtr, E_RESULT_CODE& result)
	{
		COGLVertexBuffer* pVertexBufferInstance = new (std::nothrow) COGLVertexBuffer();

		if (!pVertexBufferInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pVertexBufferInstance->Init(pGraphicsContext, usageType, totalBufferSize, pDataPtr);

		if (result != RC_OK)
		{
			delete pVertexBufferInstance;

			pVertexBufferInstance = nullptr;
		}

		return pVertexBufferInstance;
	}
}