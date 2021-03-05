#include "./../include/COGLVertexBuffer.h"
#include "./../include/COGLBuffer.h"
#include "./../include/COGLUtils.h"
#include <core/IGraphicsContext.h>
#include <memory>


namespace TDEngine2
{
	COGLVertexBuffer::COGLVertexBuffer() :
		CBaseObject()
	{
	}

	E_RESULT_CODE COGLVertexBuffer::Init(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, U32 totalBufferSize, const void* pDataPtr)
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

	void COGLVertexBuffer::Bind(U32 slot, U32 offset, U32 stride)
	{
		GL_SAFE_VOID_CALL(glBindBuffer(GL_ARRAY_BUFFER, mpBufferImpl->GetInternalData().mGLBuffer));
	}

	void COGLVertexBuffer::SetVAOHandler(GLuint vaoHandler)
	{
		mVAOHandler = vaoHandler;
	}

	const TBufferInternalData& COGLVertexBuffer::GetInternalData() const
	{
		return mpBufferImpl->GetInternalData();
	}

	U32 COGLVertexBuffer::GetSize() const
	{
		return mpBufferImpl->GetSize();
	}

	U32 COGLVertexBuffer::GetUsedSize() const
	{
		return mpBufferImpl->GetUsedSize();
	}


	TDE2_API IVertexBuffer* CreateOGLVertexBuffer(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType,
												  U32 totalBufferSize, const void* pDataPtr, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IVertexBuffer, COGLVertexBuffer, result, pGraphicsContext, usageType, totalBufferSize, pDataPtr);
	}
}