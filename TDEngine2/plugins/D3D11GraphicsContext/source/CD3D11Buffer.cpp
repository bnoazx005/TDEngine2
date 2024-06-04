#include "../include/CD3D11Buffer.h"
#include "../include/CD3D11Mappings.h"
#include "../include/CD3D11Utils.h"
#include <core/IGraphicsContext.h>
#include <memory>


#if defined (TDE2_USE_WINPLATFORM)

namespace TDEngine2
{
	static TResult<ID3D11Buffer*> CreateBufferInternal(ID3D11Device* p3dDevice, const TInitBufferParams& params, bool createEmpty = false)
	{
		D3D11_BUFFER_DESC bufferDesc {};

		U32 bufferCreationFlags = 0x0;

		switch (params.mBufferType)
		{
			case E_BUFFER_TYPE::VERTEX:
				bufferCreationFlags = D3D11_BIND_VERTEX_BUFFER;
				break;
			case E_BUFFER_TYPE::INDEX:
				bufferCreationFlags = D3D11_BIND_INDEX_BUFFER;
				break;
			case E_BUFFER_TYPE::CONSTANT:
				bufferCreationFlags = D3D11_BIND_CONSTANT_BUFFER;
				break;
			case E_BUFFER_TYPE::STRUCTURED:
				bufferCreationFlags = D3D11_BIND_SHADER_RESOURCE | (params.mIsUnorderedAccessResource ? D3D11_BIND_UNORDERED_ACCESS : 0x0);
				break;
		}

		bufferDesc.BindFlags = bufferCreationFlags;
		bufferDesc.ByteWidth = static_cast<U32>(params.mTotalBufferSize);
		bufferDesc.CPUAccessFlags = CD3D11Mappings::GetAccessFlags(params.mUsageType);
		bufferDesc.Usage = CD3D11Mappings::GetUsageType(params.mUsageType);

		if (E_BUFFER_TYPE::STRUCTURED == params.mBufferType)
		{
			bufferDesc.StructureByteStride = static_cast<UINT>(params.mElementStrideSize);

			switch (params.mStructuredBufferType)
			{
				case E_STRUCTURED_BUFFER_TYPE::DEFAULT:
					bufferDesc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
					break;
				case E_STRUCTURED_BUFFER_TYPE::RAW:
					bufferDesc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
					break;
				default:
					TDE2_UNIMPLEMENTED();
					break;
			}
		}

		D3D11_SUBRESOURCE_DATA bufferData;

		memset(&bufferData, 0, sizeof(bufferData));

		bufferData.pSysMem = createEmpty ? nullptr : params.mpDataPtr;

		ID3D11Buffer* pBuffer = nullptr;

		if (FAILED(p3dDevice->CreateBuffer(&bufferDesc, createEmpty ? nullptr : (params.mpDataPtr ? &bufferData : nullptr), &pBuffer)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

#if TDE2_DEBUG_MODE
		if (params.mName)
		{
			pBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<U32>(strlen(params.mName)), params.mName);
		}
#endif

		return Wrench::TOkValue<ID3D11Buffer*>(pBuffer);
	}


	static TResult<ID3D11ShaderResourceView*> CreateTypedBufferViewInternal(ID3D11Device* p3dDevice, ID3D11Resource* pBuffer, const TInitBufferParams& params)
	{
		if (!params.mElementStrideSize)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		ID3D11ShaderResourceView* pView = nullptr;

		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
		viewDesc.Format = DXGI_FORMAT_UNKNOWN;
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		viewDesc.Buffer.FirstElement = 0;
		viewDesc.Buffer.NumElements = static_cast<U32>(params.mTotalBufferSize / params.mElementStrideSize);

		if (FAILED(p3dDevice->CreateShaderResourceView(pBuffer, &viewDesc, &pView)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<ID3D11ShaderResourceView*>(pView);
	}


	static TResult<ID3D11UnorderedAccessView*> CreateWriteableTypedBufferViewInternal(ID3D11Device* p3dDevice, ID3D11Resource* pBuffer, const TInitBufferParams& params)
	{
		if (!params.mElementStrideSize)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		ID3D11UnorderedAccessView* pView = nullptr;

		D3D11_UNORDERED_ACCESS_VIEW_DESC viewDesc;
		viewDesc.Format = DXGI_FORMAT_UNKNOWN;
		viewDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		viewDesc.Buffer.FirstElement = 0;
		viewDesc.Buffer.Flags = 0;
		viewDesc.Buffer.NumElements = static_cast<U32>(params.mTotalBufferSize / params.mElementStrideSize);

		if (FAILED(p3dDevice->CreateUnorderedAccessView(pBuffer, &viewDesc, &pView)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<ID3D11UnorderedAccessView*>(pView);
	}


	CD3D11Buffer::CD3D11Buffer() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CD3D11Buffer::Init(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		TD3D11CtxInternalData internalD3D11Data;

#if _HAS_CXX17
		internalD3D11Data = std::get<TD3D11CtxInternalData>(pGraphicsContext->GetInternalData());
#else
		internalD3D11Data = pGraphicsContext->GetInternalData().mD3D11;
#endif

		mp3dDeviceContext = internalD3D11Data.mp3dDeviceContext;
		mp3dDevice = internalD3D11Data.mp3dDevice;

		if (!mp3dDeviceContext)
		{
			return RC_INVALID_ARGS;
		}

		mBufferSize = params.mTotalBufferSize;
		mBufferUsageType = params.mUsageType;
		mBufferType = params.mBufferType;

		E_RESULT_CODE result = _onInitInternal(params);
		if (RC_OK != result)
		{
			return result;
		}

		mInitParams = params;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D11Buffer::_onInitInternal(const TInitBufferParams& params)
	{
		auto createBufferResourceResult = CreateBufferInternal(mp3dDevice, params, false);
		if (createBufferResourceResult.HasError())
		{
			return createBufferResourceResult.GetError();
		}

		mpBufferInstance = createBufferResourceResult.Get();

		if (E_BUFFER_TYPE::STRUCTURED == params.mBufferType)
		{
			auto createViewResourceResult = CreateTypedBufferViewInternal(mp3dDevice, mpBufferInstance, params);
			if (createBufferResourceResult.HasError())
			{
				return createBufferResourceResult.GetError();
			}

			mpShaderView = createViewResourceResult.Get();
		}

		if (params.mIsUnorderedAccessResource)
		{
			auto createWritableViewResourceResult = CreateWriteableTypedBufferViewInternal(mp3dDevice, mpBufferInstance, params);
			if (createWritableViewResourceResult.HasError())
			{
				return createWritableViewResourceResult.GetError();
			}

			mpWritableShaderView = createWritableViewResourceResult.Get();
		}

		return RC_OK;
	}

	E_RESULT_CODE CD3D11Buffer::_onFreeInternal()
	{
		E_RESULT_CODE result = RC_OK;

		result = SafeReleaseCOMPtr<ID3D11Buffer>(&mpBufferInstance);

		if (mpShaderView)
		{
			result = result | SafeReleaseCOMPtr<ID3D11ShaderResourceView>(&mpShaderView);
		}

		if (mpWritableShaderView)
		{
			result = result | SafeReleaseCOMPtr<ID3D11UnorderedAccessView>(&mpWritableShaderView);
		}

		return RC_OK;
	}

	E_RESULT_CODE CD3D11Buffer::Map(E_BUFFER_MAP_TYPE mapType, USIZE offset)
	{
		D3D11_MAP innerMapType = D3D11_MAP_WRITE_DISCARD;

		switch (mapType)
		{
			case BMT_NOOVERWRITE:
				innerMapType = D3D11_MAP_WRITE_NO_OVERWRITE;
				break;
			case BMT_WRITE:
				innerMapType = D3D11_MAP_WRITE;
				break;
			case BMT_READ_WRITE:
				innerMapType = D3D11_MAP_READ_WRITE;
				break;
			case BMT_READ:
				innerMapType = D3D11_MAP_READ;
				break;
			default:
				break;
		}

		if (FAILED(mp3dDeviceContext->Map(mpBufferInstance, 0, innerMapType, 0, &mMappedBufferData)))
		{
			return RC_FAIL;
		}

		mpLockDataPtr = reinterpret_cast<void*>(reinterpret_cast<U8*>(mMappedBufferData.pData) + offset);

		return RC_OK;
	}

	void CD3D11Buffer::Unmap()
	{
		mp3dDeviceContext->Unmap(mpBufferInstance, 0);
	}

	E_RESULT_CODE CD3D11Buffer::Write(const void* pData, USIZE size)
	{
		if (size > mBufferSize)
		{
			return RC_INVALID_ARGS;
		}

		if (!mpLockDataPtr)
		{
			return RC_FAIL;
		}

		memcpy(mpLockDataPtr, pData, size);

		return RC_OK;
	}

	void* CD3D11Buffer::Read()
	{
		return mpLockDataPtr;
	}

	E_RESULT_CODE CD3D11Buffer::Resize(USIZE newSize)
	{
		auto paramsCopy = mInitParams;
		paramsCopy.mTotalBufferSize = newSize;

		if (mpLockDataPtr)
		{
			Unmap();
		}

		E_RESULT_CODE result = _onFreeInternal();
		if (RC_OK != result)
		{
			return result;
		}

		result = _onInitInternal(paramsCopy);
		if (RC_OK != result)
		{
			return result;
		}

		mBufferSize = newSize;
		mInitParams.mTotalBufferSize = mBufferSize;

		return RC_OK;
	}

	void* CD3D11Buffer::GetInternalData()
	{
		return reinterpret_cast<void*>(mpBufferInstance);
	}

	USIZE CD3D11Buffer::GetSize() const
	{
		return mBufferSize;
	}

	ID3D11DeviceContext* CD3D11Buffer::GetDeviceContext() const
	{
		return mp3dDeviceContext;
	}

	const TInitBufferParams& CD3D11Buffer::GetParams() const
	{
		return mInitParams;
	}

	ID3D11Buffer* CD3D11Buffer::GetD3D11Buffer()
	{
		return mpBufferInstance;
	}

	ID3D11ShaderResourceView* CD3D11Buffer::GetShaderView()
	{
		return mpShaderView;
	}

	ID3D11UnorderedAccessView* CD3D11Buffer::GetWriteableShaderView()
	{
		return mpWritableShaderView;
	}


	TDE2_API IBuffer* CreateD3D11Buffer(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IBuffer, CD3D11Buffer, result, pGraphicsContext, params);
	}
}

#endif