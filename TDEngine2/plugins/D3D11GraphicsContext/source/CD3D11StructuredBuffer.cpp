#include "../include/CD3D11StructuredBuffer.h"
#include "../include/CD3D11Buffer.h"
#include "../include/CD3D11Mappings.h"
#include <core/IGraphicsContext.h>
#include <memory>


#if defined (TDE2_USE_WINPLATFORM)

namespace TDEngine2
{

	static TResult<ID3D11UnorderedAccessView*> CreateUnorderedAccessView(ID3D11Buffer* pBuffer, ID3D11Device* p3dDevice, E_STRUCTURED_BUFFER_TYPE type, U32 numElements)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC viewDesc = {};

		viewDesc.Format = (E_STRUCTURED_BUFFER_TYPE::DEFAULT == type) ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_R32_TYPELESS;
		viewDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;

		viewDesc.Buffer.FirstElement = 0;
		viewDesc.Buffer.Flags = 0;

		switch (type)
		{
			case E_STRUCTURED_BUFFER_TYPE::RAW:
				viewDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
				break;
			case E_STRUCTURED_BUFFER_TYPE::APPENDABLE:
				viewDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
				break;
		}

		viewDesc.Buffer.NumElements = numElements;

		ID3D11UnorderedAccessView* pView = nullptr;

		if (FAILED(p3dDevice->CreateUnorderedAccessView(pBuffer, &viewDesc, &pView)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<ID3D11UnorderedAccessView*>(pView);
	}


	static TResult<ID3D11ShaderResourceView*> CreateShaderResourceView(ID3D11Buffer* pBuffer, ID3D11Device* p3dDevice, E_STRUCTURED_BUFFER_TYPE type, U32 numElements)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};

		viewDesc.Format = (E_STRUCTURED_BUFFER_TYPE::DEFAULT == type) ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_R32_TYPELESS;
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		
		viewDesc.BufferEx.FirstElement = 0;
		viewDesc.BufferEx.Flags = (E_STRUCTURED_BUFFER_TYPE::RAW == type) ? D3D11_BUFFEREX_SRV_FLAG_RAW : 0;
		viewDesc.BufferEx.NumElements = numElements;

		ID3D11ShaderResourceView* pView = nullptr;

		if (FAILED(p3dDevice->CreateShaderResourceView(pBuffer, &viewDesc, &pView)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<ID3D11ShaderResourceView*>(pView);
	}


	CD3D11StructuredBuffer::CD3D11StructuredBuffer() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CD3D11StructuredBuffer::Init(const TStructuredBuffersInitParams& params)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mStructuredElementStride = params.mElementStride;

		IGraphicsContext* pGraphicsContext = params.mpGraphicsContext;

		E_RESULT_CODE result = RC_OK;

		mpBufferImpl = CreateD3D11Buffer(
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

		TD3D11CtxInternalData internalD3D11Data;

#if _HAS_CXX17
		internalD3D11Data = std::get<TD3D11CtxInternalData>(pGraphicsContext->GetInternalData());
#else
		internalD3D11Data = pGraphicsContext->GetInternalData().mD3D11;
#endif

		mp3dDeviceContext = internalD3D11Data.mp3dDeviceContext;

		auto pInternalBufferD3D11 = mpBufferImpl->GetInternalData().mpD3D11Buffer; 

		auto createSRVResult = CreateShaderResourceView(pInternalBufferD3D11, internalD3D11Data.mp3dDevice, params.mBufferType, params.mElementsCount);
		if (createSRVResult.HasError())
		{
			return createSRVResult.GetError();
		}

		mpShaderResourceView = createSRVResult.Get();

		if (params.mIsWriteable) /// \note If the buffer is writeable create UAV view
		{
			auto createUAVResult = CreateUnorderedAccessView(pInternalBufferD3D11, internalD3D11Data.mp3dDevice, params.mBufferType, params.mElementsCount);
			if (createUAVResult.HasError())
			{
				return createUAVResult.GetError();
			}

			mpUavResourceView = createUAVResult.Get();
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D11StructuredBuffer::Map(E_BUFFER_MAP_TYPE mapType)
	{
		return mpBufferImpl->Map(mapType);
	}

	void CD3D11StructuredBuffer::Unmap()
	{
		return mpBufferImpl->Unmap();
	}

	E_RESULT_CODE CD3D11StructuredBuffer::Write(const void* pData, USIZE size)
	{
		return mpBufferImpl->Write(pData, size);
	}

	void* CD3D11StructuredBuffer::Read()
	{
		return mpBufferImpl->Read();
	}

	void CD3D11StructuredBuffer::Bind(U32 slot)
	{
		mp3dDeviceContext->VSSetShaderResources(slot, 1, &mpShaderResourceView);
		mp3dDeviceContext->PSSetShaderResources(slot, 1, &mpShaderResourceView);
		mp3dDeviceContext->GSSetShaderResources(slot, 1, &mpShaderResourceView);
		mp3dDeviceContext->CSSetShaderResources(slot, 1, &mpShaderResourceView);

		if (mIsWriteable)
		{
			mp3dDeviceContext->CSSetUnorderedAccessViews(slot, 1, &mpUavResourceView, nullptr);
		}
	}

	const TBufferInternalData& CD3D11StructuredBuffer::GetInternalData() const
	{
		return mpBufferImpl->GetInternalData();
	}

	USIZE CD3D11StructuredBuffer::GetSize() const
	{
		return mpBufferImpl->GetSize();
	}

	USIZE CD3D11StructuredBuffer::GetUsedSize() const
	{
		return mpBufferImpl->GetUsedSize();
	}

	USIZE CD3D11StructuredBuffer::GetStrideSize() const
	{
		return mStructuredElementStride;
	}


	TDE2_API IStructuredBuffer* CreateD3D11StructuredBuffer(const TStructuredBuffersInitParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IStructuredBuffer, CD3D11StructuredBuffer, result, params);
	}
}

#endif