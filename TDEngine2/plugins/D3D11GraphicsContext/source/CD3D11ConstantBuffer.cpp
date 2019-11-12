#include "./../include/CD3D11ConstantBuffer.h"
#include "./../include/CD3D11Buffer.h"
#include <core/IGraphicsContext.h>
#include <memory>


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	CD3D11ConstantBuffer::CD3D11ConstantBuffer() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CD3D11ConstantBuffer::Init(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, U32 totalBufferSize, const void* pDataPtr)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		mpBufferImpl = CreateD3D11Buffer(pGraphicsContext, usageType, BT_CONSTANT_BUFFER, totalBufferSize, pDataPtr, result);

		if (result != RC_OK)
		{
			return result;
		}

		mp3dDeviceContext = dynamic_cast<CD3D11Buffer*>(mpBufferImpl)->GetDeviceContext();

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D11ConstantBuffer::Free()
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

	E_RESULT_CODE CD3D11ConstantBuffer::Map(E_BUFFER_MAP_TYPE mapType)
	{
		return mpBufferImpl->Map(mapType);
	}

	void CD3D11ConstantBuffer::Unmap()
	{
		return mpBufferImpl->Unmap();
	}

	E_RESULT_CODE CD3D11ConstantBuffer::Write(const void* pData, U32 size)
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

	U32 CD3D11ConstantBuffer::GetSize() const
	{
		return mpBufferImpl->GetSize();
	}


	TDE2_API IConstantBuffer* CreateD3D11ConstantBuffer(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType,
		U32 totalBufferSize, const void* pDataPtr, E_RESULT_CODE& result)
	{
		CD3D11ConstantBuffer* pConstantBufferInstance = new (std::nothrow) CD3D11ConstantBuffer();

		if (!pConstantBufferInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pConstantBufferInstance->Init(pGraphicsContext, usageType, totalBufferSize, pDataPtr);

		if (result != RC_OK)
		{
			delete pConstantBufferInstance;

			pConstantBufferInstance = nullptr;
		}

		return pConstantBufferInstance;
	}
}

#endif