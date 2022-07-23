#include "./../include/CD3D11ConstantBuffer.h"
#include "./../include/CD3D11Buffer.h"
#include <core/IGraphicsContext.h>
#include <memory>


#if defined (TDE2_USE_WINPLATFORM)

namespace TDEngine2
{
	CD3D11ConstantBuffer::CD3D11ConstantBuffer() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CD3D11ConstantBuffer::Init(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, USIZE totalBufferSize, const void* pDataPtr)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		mpBufferImpl = CreateD3D11Buffer({ pGraphicsContext, usageType, E_BUFFER_TYPE::BT_CONSTANT_BUFFER, totalBufferSize, pDataPtr }, result);

		if (result != RC_OK)
		{
			return result;
		}

		mp3dDeviceContext = dynamic_cast<CD3D11Buffer*>(mpBufferImpl)->GetDeviceContext();

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D11ConstantBuffer::_onFreeInternal()
	{
		return mpBufferImpl->Free();
	}

	E_RESULT_CODE CD3D11ConstantBuffer::Map(E_BUFFER_MAP_TYPE mapType)
	{
		return mpBufferImpl->Map(mapType);
	}

	void CD3D11ConstantBuffer::Unmap()
	{
		return mpBufferImpl->Unmap();
	}

	E_RESULT_CODE CD3D11ConstantBuffer::Write(const void* pData, USIZE size)
	{
		return mpBufferImpl->Write(pData, size);
	}

	void* CD3D11ConstantBuffer::Read()
	{
		return mpBufferImpl->Read();
	}

	void CD3D11ConstantBuffer::Bind(U32 slot)
	{
		if (!mIsInitialized)
		{
			return;
		}

		ID3D11Buffer* pInternalBuffer = mpBufferImpl->GetInternalData().mpD3D11Buffer;

		mp3dDeviceContext->VSSetConstantBuffers(slot, 1, &pInternalBuffer);
		mp3dDeviceContext->PSSetConstantBuffers(slot, 1, &pInternalBuffer);
		mp3dDeviceContext->GSSetConstantBuffers(slot, 1, &pInternalBuffer);
		mp3dDeviceContext->CSSetConstantBuffers(slot, 1, &pInternalBuffer);

		mCurrUsedSlot = slot;
	}

	void CD3D11ConstantBuffer::Unbind()
	{
		if (!mIsInitialized || !mCurrUsedSlot)
		{
			return;
		}
		
		/*mp3dDeviceContext->VSGetConstantBuffers(mCurrUsedSlot, 1, nullptr);
		mp3dDeviceContext->PSGetConstantBuffers(mCurrUsedSlot, 1, nullptr);
		mp3dDeviceContext->GSGetConstantBuffers(mCurrUsedSlot, 1, nullptr);
*/
		mCurrUsedSlot = 0;
	}


	const TBufferInternalData& CD3D11ConstantBuffer::GetInternalData() const
	{
		return mpBufferImpl->GetInternalData();
	}

	USIZE CD3D11ConstantBuffer::GetSize() const
	{
		return mpBufferImpl->GetSize();
	}

	USIZE CD3D11ConstantBuffer::GetUsedSize() const
	{
		return mpBufferImpl->GetUsedSize();
	}


	TDE2_API IConstantBuffer* CreateD3D11ConstantBuffer(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType,
		USIZE totalBufferSize, const void* pDataPtr, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IConstantBuffer, CD3D11ConstantBuffer, result, pGraphicsContext, usageType, totalBufferSize, pDataPtr);
	}
}

#endif