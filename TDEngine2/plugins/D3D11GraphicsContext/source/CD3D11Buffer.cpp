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
				bufferCreationFlags = params.mIsUnorderedAccessResource ? D3D11_BIND_UNORDERED_ACCESS : D3D11_BIND_SHADER_RESOURCE;
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
		pBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<U32>(params.mName.length()), params.mName.c_str());
#endif

		return Wrench::TOkValue<ID3D11Buffer*>(pBuffer);
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

		auto createBufferResourceResult = CreateBufferInternal(internalD3D11Data.mp3dDevice, params, false);
		if (createBufferResourceResult.HasError())
		{
			return createBufferResourceResult.GetError();
		}

		mpBufferInstance = createBufferResourceResult.Get();

		mInitParams = params;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D11Buffer::_onFreeInternal()
	{
		E_RESULT_CODE result = RC_OK;

		if ((result = SafeReleaseCOMPtr<ID3D11Buffer>(&mpBufferInstance)) != RC_OK)
		{
			return result;
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

		auto createBufferHandleResult = CreateBufferInternal(mp3dDevice, paramsCopy, true);
		if (createBufferHandleResult.HasError())
		{
			return createBufferHandleResult.GetError();
		}

		if (mpLockDataPtr)
		{
			Unmap();
		}

		E_RESULT_CODE result = SafeReleaseCOMPtr<ID3D11Buffer>(&mpBufferInstance);
		if (RC_OK != result)
		{
			return result;
		}

		mpBufferInstance = createBufferHandleResult.Get();

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


	TDE2_API IBuffer* CreateD3D11Buffer(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IBuffer, CD3D11Buffer, result, pGraphicsContext, params);
	}
}

#endif