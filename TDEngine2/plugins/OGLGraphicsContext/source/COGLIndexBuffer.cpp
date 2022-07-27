#include "../include/COGLIndexBuffer.h"
#include "../include/COGLBuffer.h"
#include "../include/COGLUtils.h"
#include <core/IGraphicsContext.h>
#include <memory>


namespace TDEngine2
{
	COGLIndexBuffer::COGLIndexBuffer() :
		CBaseObject()
	{
	}

	E_RESULT_CODE COGLIndexBuffer::Init(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, USIZE totalBufferSize,
										E_INDEX_FORMAT_TYPE indexFormatType, const void* pDataPtr)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIndexFormatType = indexFormatType;

		E_RESULT_CODE result = RC_OK;

		mpBufferImpl = CreateOGLBuffer({ pGraphicsContext, usageType, E_BUFFER_TYPE::BT_INDEX_BUFFER, totalBufferSize, pDataPtr }, result);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE COGLIndexBuffer::_onFreeInternal()
	{
		return mpBufferImpl->Free();
	}

	E_RESULT_CODE COGLIndexBuffer::Map(E_BUFFER_MAP_TYPE mapType)
	{
		return mpBufferImpl->Map(mapType);
	}

	void COGLIndexBuffer::Unmap()
	{
		return mpBufferImpl->Unmap();
	}

	E_RESULT_CODE COGLIndexBuffer::Write(const void* pData, USIZE size)
	{
		return mpBufferImpl->Write(pData, size);
	}

	void* COGLIndexBuffer::Read()
	{
		return mpBufferImpl->Read();
	}

	void COGLIndexBuffer::Bind(U32 offset)
	{
		GL_SAFE_VOID_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mpBufferImpl->GetInternalData().mGLBuffer));
	}

	const TBufferInternalData& COGLIndexBuffer::GetInternalData() const
	{
		return mpBufferImpl->GetInternalData();
	}

	USIZE COGLIndexBuffer::GetSize() const
	{
		return mpBufferImpl->GetSize();
	}

	USIZE COGLIndexBuffer::GetUsedSize() const
	{
		return mpBufferImpl->GetUsedSize();
	}

	E_INDEX_FORMAT_TYPE COGLIndexBuffer::GetIndexFormat() const
	{
		return mIndexFormatType;
	}


	TDE2_API IIndexBuffer* CreateOGLIndexBuffer(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, E_INDEX_FORMAT_TYPE indexFormatType,
		USIZE totalBufferSize, const void* pDataPtr, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IIndexBuffer, COGLIndexBuffer, result, pGraphicsContext, usageType, totalBufferSize, indexFormatType, pDataPtr);
	}
}