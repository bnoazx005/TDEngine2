#include "./../include/CVulkanConstantBuffer.h"
#include "./../include/CVulkanBuffer.h"
#include "./../include/CVulkanUtils.h"
#include <core/IGraphicsContext.h>
#include <memory>


namespace TDEngine2
{
	CVulkanConstantBuffer::CVulkanConstantBuffer() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CVulkanConstantBuffer::Init(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, USIZE totalBufferSize, const void* pDataPtr)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		mpBufferImpl = CreateVulkanBuffer({ pGraphicsContext, usageType, E_BUFFER_TYPE::BT_CONSTANT_BUFFER, totalBufferSize, pDataPtr }, result);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CVulkanConstantBuffer::_onFreeInternal()
	{
		return mpBufferImpl->Free();
	}

	E_RESULT_CODE CVulkanConstantBuffer::Map(E_BUFFER_MAP_TYPE mapType)
	{
		return mpBufferImpl->Map(mapType);
	}

	void CVulkanConstantBuffer::Unmap()
	{
		return mpBufferImpl->Unmap();
	}

	E_RESULT_CODE CVulkanConstantBuffer::Write(const void* pData, USIZE size)
	{
		return mpBufferImpl->Write(pData, size);
	}

	void* CVulkanConstantBuffer::Read()
	{
		return mpBufferImpl->Read();
	}

	void CVulkanConstantBuffer::Bind(U32 slot)
	{
		if (!mIsInitialized)
		{
			return;
		}

		mCurrUsedSlot = slot;

		//GL_SAFE_VOID_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, slot, mpBufferImpl->GetInternalData().mGLBuffer));
	}

	void CVulkanConstantBuffer::Unbind()
	{
		if (!mIsInitialized && !mCurrUsedSlot)
		{
			return;
		}

		//GL_SAFE_VOID_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, mCurrUsedSlot, 0));

		mCurrUsedSlot = 0;
	}

	const TBufferInternalData& CVulkanConstantBuffer::GetInternalData() const
	{
		return mpBufferImpl->GetInternalData();
	}

	USIZE CVulkanConstantBuffer::GetSize() const
	{
		return mpBufferImpl->GetSize();
	}

	USIZE CVulkanConstantBuffer::GetUsedSize() const
	{
		return mpBufferImpl->GetUsedSize();
	}


	TDE2_API IConstantBuffer* CreateVulkanConstantBuffer(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType,
													  USIZE totalBufferSize, const void* pDataPtr, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IConstantBuffer, CVulkanConstantBuffer, result, pGraphicsContext, usageType, totalBufferSize, pDataPtr);
	}
}