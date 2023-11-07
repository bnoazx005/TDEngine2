#include "../include/CVulkanStructuredBuffer.h"
#include "../include/CVulkanBuffer.h"
#include "../include/CVulkanUtils.h"
#include <core/IGraphicsContext.h>
#include <memory>


namespace TDEngine2
{
	CVulkanStructuredBuffer::CVulkanStructuredBuffer() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CVulkanStructuredBuffer::Init(const TStructuredBuffersInitParams& params)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mStructuredElementStride = params.mElementStride;

		IGraphicsContext* pGraphicsContext = params.mpGraphicsContext;

		E_RESULT_CODE result = RC_OK;

		mpBufferImpl = CreateVulkanBuffer(
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

	E_RESULT_CODE CVulkanStructuredBuffer::Map(E_BUFFER_MAP_TYPE mapType)
	{
		return mpBufferImpl->Map(mapType);
	}

	void CVulkanStructuredBuffer::Unmap()
	{
		return mpBufferImpl->Unmap();
	}

	E_RESULT_CODE CVulkanStructuredBuffer::Write(const void* pData, USIZE size)
	{
		return mpBufferImpl->Write(pData, size);
	}

	void* CVulkanStructuredBuffer::Read()
	{
		return mpBufferImpl->Read();
	}

	void CVulkanStructuredBuffer::Bind(U32 slot)
	{
		if (!mIsInitialized)
		{
			return;
		}

		mCurrUsedSlot = slot;

		//GL_SAFE_VOID_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, mpBufferImpl->GetInternalData().mGLBuffer));
	}

	const TBufferInternalData& CVulkanStructuredBuffer::GetInternalData() const
	{
		return mpBufferImpl->GetInternalData();
	}

	USIZE CVulkanStructuredBuffer::GetSize() const
	{
		return mpBufferImpl->GetSize();
	}

	USIZE CVulkanStructuredBuffer::GetUsedSize() const
	{
		return mpBufferImpl->GetUsedSize();
	}

	USIZE CVulkanStructuredBuffer::GetStrideSize() const
	{
		return mStructuredElementStride;
	}


	TDE2_API IStructuredBuffer* CreateVulkanStructuredBuffer(const TStructuredBuffersInitParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IStructuredBuffer, CVulkanStructuredBuffer, result, params);
	}
}