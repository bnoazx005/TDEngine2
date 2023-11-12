#include "../include/CD3D11Buffer.h"
#include "../include/CD3D11Mappings.h"
#include "../include/CD3D11Utils.h"
#include <core/IGraphicsContext.h>
#include <memory>


#if defined (TDE2_USE_WINPLATFORM)

namespace TDEngine2
{
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

		if (!mp3dDeviceContext)
		{
			return RC_INVALID_ARGS;
		}

		mBufferSize = params.mTotalBufferSize;
		mUsedBytesSize = 0;

		mBufferUsageType = params.mUsageType;

		mBufferType = params.mBufferType;

		D3D11_BUFFER_DESC bufferDesc;

		memset(&bufferDesc, 0, sizeof(bufferDesc));

		U32 bufferCreationFlags = 0x0;

		switch (mBufferType)
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
		bufferDesc.ByteWidth = static_cast<U32>(mBufferSize);
		bufferDesc.CPUAccessFlags = CD3D11Mappings::GetAccessFlags(mBufferUsageType);
		bufferDesc.Usage = CD3D11Mappings::GetUsageType(mBufferUsageType);
		
		if (E_BUFFER_TYPE::STRUCTURED == mBufferType)
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

		bufferData.pSysMem = params.mpDataPtr;

		ID3D11Device* p3dDevice = internalD3D11Data.mp3dDevice;

		if (FAILED(p3dDevice->CreateBuffer(&bufferDesc, (params.mpDataPtr ? &bufferData : nullptr), &mpBufferInstance)))
		{
			return RC_FAIL;
		}

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

	E_RESULT_CODE CD3D11Buffer::Map(E_BUFFER_MAP_TYPE mapType)
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

		void* pMappedBufferDataPtr = mMappedBufferData.pData;

		if (!mMappedBufferData.pData)
		{
			return RC_FAIL;
		}

		memcpy(pMappedBufferDataPtr, pData, size);

		mUsedBytesSize += size;

		return RC_OK;
	}

	void* CD3D11Buffer::Read()
	{
		return mMappedBufferData.pData;
	}

	void* CD3D11Buffer::GetInternalData()
	{
		return reinterpret_cast<void*>(mpBufferInstance);
	}

	USIZE CD3D11Buffer::GetSize() const
	{
		return mBufferSize;
	}

	USIZE CD3D11Buffer::GetUsedSize() const
	{
		return mUsedBytesSize;
	}

	ID3D11DeviceContext* CD3D11Buffer::GetDeviceContext() const
	{
		return mp3dDeviceContext;
	}

	const TInitBufferParams& CD3D11Buffer::GetParams() const
	{
		return mInitParams;
	}


	TDE2_API IBuffer* CreateD3D11Buffer(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IBuffer, CD3D11Buffer, result, pGraphicsContext, params);
	}
}

#endif