#include "../include/CD3D11IndexBuffer.h"
#include "../include/CD3D11Buffer.h"
#include "../include/CD3D11Mappings.h"
#include <core/IGraphicsContext.h>
#include <memory>


#if defined (TDE2_USE_WINPLATFORM)

namespace TDEngine2
{
	CD3D11IndexBuffer::CD3D11IndexBuffer() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CD3D11IndexBuffer::Init(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, USIZE totalBufferSize,
										  E_INDEX_FORMAT_TYPE indexFormatType, const void* pDataPtr)
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

		TD3D11CtxInternalData internalD3D11Data;

#if _HAS_CXX17
		internalD3D11Data = std::get<TD3D11CtxInternalData>(pGraphicsContext->GetInternalData());
#else
		internalD3D11Data = pGraphicsContext->GetInternalData().mD3D11;
#endif

		mp3dDeviceContext = internalD3D11Data.mp3dDeviceContext;

		mpInternalIndexBuffer = mpBufferImpl->GetInternalData().mpD3D11Buffer; /// cache ID3D11Buffer to avoid extra calls in Bind method

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D11IndexBuffer::_onFreeInternal()
	{
		return mpBufferImpl->Free();
	}

	E_RESULT_CODE CD3D11IndexBuffer::Map(E_BUFFER_MAP_TYPE mapType)
	{
		return mpBufferImpl->Map(mapType);
	}

	void CD3D11IndexBuffer::Unmap()
	{
		return mpBufferImpl->Unmap();
	}

	E_RESULT_CODE CD3D11IndexBuffer::Write(const void* pData, USIZE size)
	{
		return mpBufferImpl->Write(pData, size);
	}

	void* CD3D11IndexBuffer::Read()
	{
		return mpBufferImpl->Read();
	}

	void CD3D11IndexBuffer::Bind(U32 offset)
	{
		if (!mIsInitialized)
		{
			return;
		}

		mp3dDeviceContext->IASetIndexBuffer(mpInternalIndexBuffer, CD3D11Mappings::GetIndexFormat(mIndexFormatType), offset);
	}

	const TBufferInternalData& CD3D11IndexBuffer::GetInternalData() const
	{
		return mpBufferImpl->GetInternalData();
	}

	USIZE CD3D11IndexBuffer::GetSize() const
	{
		return mpBufferImpl->GetSize();
	}

	USIZE CD3D11IndexBuffer::GetUsedSize() const
	{
		return mpBufferImpl->GetUsedSize();
	}
	
	E_INDEX_FORMAT_TYPE CD3D11IndexBuffer::GetIndexFormat() const
	{
		return mIndexFormatType;
	}


	TDE2_API IIndexBuffer* CreateD3D11IndexBuffer(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, E_INDEX_FORMAT_TYPE indexFormatType,
												  USIZE totalBufferSize, const void* pDataPtr, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IIndexBuffer, CD3D11IndexBuffer, result, pGraphicsContext, usageType, totalBufferSize, indexFormatType, pDataPtr);
	}
}

#endif