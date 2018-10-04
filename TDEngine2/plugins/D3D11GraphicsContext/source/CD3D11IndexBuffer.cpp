#include "./../include/CD3D11IndexBuffer.h"
#include "./../include/CD3D11Buffer.h"
#include <core/IGraphicsContext.h>
#include <memory>


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	CD3D11IndexBuffer::CD3D11IndexBuffer() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CD3D11IndexBuffer::Init(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, U32 totalBufferSize,
										  E_INDEX_FORMAT_TYPE indexFormatType, void* pDataPtr)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIndexFormatType = indexFormatType;

		E_RESULT_CODE result = RC_OK;

		mpBufferImpl = CreateD3D11Buffer(pGraphicsContext, usageType, BT_INDEX_BUFFER, totalBufferSize, pDataPtr, result);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D11IndexBuffer::Free()
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

	E_RESULT_CODE CD3D11IndexBuffer::Map(E_BUFFER_MAP_TYPE mapType)
	{
		return mpBufferImpl->Map(mapType);
	}

	void CD3D11IndexBuffer::Unmap()
	{
		return mpBufferImpl->Unmap();
	}

	E_RESULT_CODE CD3D11IndexBuffer::Write(const void* pData, U32 size)
	{
		return mpBufferImpl->Write(pData, size);
	}

	void* CD3D11IndexBuffer::Read()
	{
		return mpBufferImpl->Read();
	}

	const TBufferInternalData& CD3D11IndexBuffer::GetInternalData() const
	{
		return mpBufferImpl->GetInternalData();
	}

	U32 CD3D11IndexBuffer::GetSize() const
	{
		return mpBufferImpl->GetSize();
	}


	TDE2_API IIndexBuffer* CreateD3D11IndexBuffer(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, E_INDEX_FORMAT_TYPE indexFormatType,
												  U32 totalBufferSize, void* pDataPtr, E_RESULT_CODE& result)
	{
		IIndexBuffer* pIndexBufferInstance = new (std::nothrow) CD3D11IndexBuffer();

		if (!pIndexBufferInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pIndexBufferInstance->Init(pGraphicsContext, usageType, totalBufferSize, indexFormatType, pDataPtr);

		return pIndexBufferInstance;
	}
}

#endif