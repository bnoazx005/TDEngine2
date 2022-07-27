#include "../include/COGLStructuredBuffer.h"
#include "../include/COGLBuffer.h"
#include "../include/COGLUtils.h"
#include <core/IGraphicsContext.h>
#include <memory>


namespace TDEngine2
{
	COGLStructuredBuffer::COGLStructuredBuffer() :
		CBaseObject()
	{
	}

	E_RESULT_CODE COGLStructuredBuffer::Init(const TStructuredBuffersInitParams& params)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mStructuredElementStride = params.mElementStride;

		IGraphicsContext* pGraphicsContext = params.mpGraphicsContext;

		E_RESULT_CODE result = RC_OK;

		mpBufferImpl = CreateOGLBuffer(
			{
				pGraphicsContext,
				params.mUsageType,
				E_BUFFER_TYPE::BT_STRUCTURED_BUFFER,
				params.mElementsCount * params.mElementStride,
				params.mpInitialData,
				params.mInitialDataSize,
				params.mIsWriteable,
				params.mElementStride,
				params.mBufferType
			}, result);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE COGLStructuredBuffer::Map(E_BUFFER_MAP_TYPE mapType)
	{
		return mpBufferImpl->Map(mapType);
	}

	void COGLStructuredBuffer::Unmap()
	{
		return mpBufferImpl->Unmap();
	}

	E_RESULT_CODE COGLStructuredBuffer::Write(const void* pData, USIZE size)
	{
		return mpBufferImpl->Write(pData, size);
	}

	void* COGLStructuredBuffer::Read()
	{
		return mpBufferImpl->Read();
	}

	void COGLStructuredBuffer::Bind(U32 slot)
	{
		if (!mIsInitialized)
		{
			return;
		}

		mCurrUsedSlot = slot;

		GL_SAFE_VOID_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, mpBufferImpl->GetInternalData().mGLBuffer));
	}

	const TBufferInternalData& COGLStructuredBuffer::GetInternalData() const
	{
		return mpBufferImpl->GetInternalData();
	}

	USIZE COGLStructuredBuffer::GetSize() const
	{
		return mpBufferImpl->GetSize();
	}

	USIZE COGLStructuredBuffer::GetUsedSize() const
	{
		return mpBufferImpl->GetUsedSize();
	}

	USIZE COGLStructuredBuffer::GetStrideSize() const
	{
		return mStructuredElementStride;
	}


	TDE2_API IStructuredBuffer* CreateOGLStructuredBuffer(const TStructuredBuffersInitParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IStructuredBuffer, COGLStructuredBuffer, result, params);
	}
}