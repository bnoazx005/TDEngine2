#include "../include/CVulkanIndexBuffer.h"
#include "../include/CVulkanBuffer.h"
#include "../include/CVulkanUtils.h"
#include <core/IGraphicsContext.h>
#include <memory>


namespace TDEngine2
{
	CVulkanIndexBuffer::CVulkanIndexBuffer() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CVulkanIndexBuffer::Init(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, USIZE totalBufferSize,
										E_INDEX_FORMAT_TYPE indexFormatType, const void* pDataPtr)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIndexFormatType = indexFormatType;

		E_RESULT_CODE result = RC_OK;

		mpBufferImpl = CreateVulkanBuffer({ pGraphicsContext, usageType, E_BUFFER_TYPE::BT_INDEX_BUFFER, totalBufferSize, pDataPtr }, result);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CVulkanIndexBuffer::_onFreeInternal()
	{
		return mpBufferImpl->Free();
	}

	E_RESULT_CODE CVulkanIndexBuffer::Map(E_BUFFER_MAP_TYPE mapType)
	{
		return mpBufferImpl->Map(mapType);
	}

	void CVulkanIndexBuffer::Unmap()
	{
		return mpBufferImpl->Unmap();
	}

	E_RESULT_CODE CVulkanIndexBuffer::Write(const void* pData, USIZE size)
	{
		return mpBufferImpl->Write(pData, size);
	}

	void* CVulkanIndexBuffer::Read()
	{
		return mpBufferImpl->Read();
	}

	void CVulkanIndexBuffer::Bind(U32 offset)
	{
		//GL_SAFE_VOID_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mpBufferImpl->GetInternalData().mGLBuffer));
	}

	const TBufferInternalData& CVulkanIndexBuffer::GetInternalData() const
	{
		return mpBufferImpl->GetInternalData();
	}

	USIZE CVulkanIndexBuffer::GetSize() const
	{
		return mpBufferImpl->GetSize();
	}

	USIZE CVulkanIndexBuffer::GetUsedSize() const
	{
		return mpBufferImpl->GetUsedSize();
	}

	E_INDEX_FORMAT_TYPE CVulkanIndexBuffer::GetIndexFormat() const
	{
		return mIndexFormatType;
	}


	TDE2_API IIndexBuffer* CreateVulkanIndexBuffer(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, E_INDEX_FORMAT_TYPE indexFormatType,
		USIZE totalBufferSize, const void* pDataPtr, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IIndexBuffer, CVulkanIndexBuffer, result, pGraphicsContext, usageType, totalBufferSize, indexFormatType, pDataPtr);
	}
}