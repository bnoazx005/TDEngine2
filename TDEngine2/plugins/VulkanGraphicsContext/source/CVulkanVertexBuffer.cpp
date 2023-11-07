#include "./../include/CVulkanVertexBuffer.h"
#include "./../include/CVulkanBuffer.h"
#include "./../include/CVulkanUtils.h"
#include <core/IGraphicsContext.h>
#include <memory>


namespace TDEngine2
{
	CVulkanVertexBuffer::CVulkanVertexBuffer() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CVulkanVertexBuffer::Init(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, USIZE totalBufferSize, const void* pDataPtr)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		mpBufferImpl = CreateVulkanBuffer({ pGraphicsContext, usageType, E_BUFFER_TYPE::BT_VERTEX_BUFFER, totalBufferSize, pDataPtr }, result);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CVulkanVertexBuffer::_onFreeInternal()
	{
		return mpBufferImpl->Free();
	}

	E_RESULT_CODE CVulkanVertexBuffer::Map(E_BUFFER_MAP_TYPE mapType)
	{
		return mpBufferImpl->Map(mapType);
	}

	void CVulkanVertexBuffer::Unmap()
	{
		return mpBufferImpl->Unmap();
	}

	E_RESULT_CODE CVulkanVertexBuffer::Write(const void* pData, USIZE size)
	{
		return mpBufferImpl->Write(pData, size);
	}

	void* CVulkanVertexBuffer::Read()
	{
		return mpBufferImpl->Read();
	}

	void CVulkanVertexBuffer::Bind(U32 slot, U32 offset, U32 stride)
	{
		//GL_SAFE_VOID_CALL(glBindBuffer(GL_ARRAY_BUFFER, mpBufferImpl->GetInternalData().mGLBuffer));
	}

	void CVulkanVertexBuffer::SetVAOHandler(GLuint vaoHandler)
	{
		mVAOHandler = vaoHandler;
	}

	const TBufferInternalData& CVulkanVertexBuffer::GetInternalData() const
	{
		return mpBufferImpl->GetInternalData();
	}

	USIZE CVulkanVertexBuffer::GetSize() const
	{
		return mpBufferImpl->GetSize();
	}

	USIZE CVulkanVertexBuffer::GetUsedSize() const
	{
		return mpBufferImpl->GetUsedSize();
	}


	TDE2_API IVertexBuffer* CreateVulkanVertexBuffer(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType,
												  USIZE totalBufferSize, const void* pDataPtr, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IVertexBuffer, CVulkanVertexBuffer, result, pGraphicsContext, usageType, totalBufferSize, pDataPtr);
	}
}