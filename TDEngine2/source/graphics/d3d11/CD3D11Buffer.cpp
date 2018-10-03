#include "./../../../include/graphics/d3d11/CD3D11Buffer.h"
#include "./../../../include/graphics/d3d11/CD3D11Mappings.h"
#include "./../../../include/core/IGraphicsContext.h"
#include <memory>


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	CD3D11Buffer::CD3D11Buffer() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CD3D11Buffer::Init(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, E_BUFFER_TYPE bufferType, U32 totalBufferSize, 
									 void* pDataPtr)
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

		mBufferSize = totalBufferSize;

		mBufferUsageType = usageType;

		mBufferType = bufferType;

		D3D11_BUFFER_DESC bufferDesc;

		memset(&bufferDesc, 0, sizeof(bufferDesc));

		U32 bufferCreationFlags = 0x0;

		switch (usageType)
		{
			case BT_VERTEX_BUFFER:
				bufferCreationFlags = D3D11_BIND_VERTEX_BUFFER;
				break;
			case BT_INDEX_BUFFER:
				bufferCreationFlags = D3D11_BIND_INDEX_BUFFER;
				break;
			case BT_CONSTANT_BUFFER:
				bufferCreationFlags = D3D11_BIND_CONSTANT_BUFFER;
				break;
		}

		bufferDesc.BindFlags      = bufferCreationFlags;
		bufferDesc.ByteWidth      = totalBufferSize;
		bufferDesc.CPUAccessFlags = CD3D11Mappings::GetAccessFlags(mBufferUsageType);
		bufferDesc.Usage          = CD3D11Mappings::GetUsageType(mBufferUsageType);

		D3D11_SUBRESOURCE_DATA bufferData;

		memset(&bufferData, 0, sizeof(bufferData));

		bufferData.pSysMem = pDataPtr;

		ID3D11Device* p3dDevice = internalD3D11Data.mp3dDevice;
		
		if (FAILED(p3dDevice->CreateBuffer(&bufferDesc, (pDataPtr ? &bufferData : nullptr), &mpBufferInstance)))
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D11Buffer::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (FAILED(mpBufferInstance->Release()))
		{
			return RC_FAIL;
		}

		delete this;

		mIsInitialized = false;

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
	
	E_RESULT_CODE CD3D11Buffer::Write(const void* pData, U32 size)
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
	}

	void* CD3D11Buffer::Read()
	{
		return mMappedBufferData.pData;
	}
	
	const TBufferInternalData& CD3D11Buffer::GetInternalData() const
	{
		return mBufferInternalData;
	}
	
	U32 CD3D11Buffer::GetSize() const
	{
		return mBufferSize;
	}


	TDE2_API IBuffer* CreateD3D11Buffer(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, E_BUFFER_TYPE bufferType,
										U32 totalBufferSize, void* pDataPtr, E_RESULT_CODE& result)
	{
		CD3D11Buffer* pGraphicsContextInstance = new (std::nothrow) CD3D11Buffer();

		if (!pGraphicsContextInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pGraphicsContextInstance->Init(pGraphicsContext, usageType, bufferType, totalBufferSize, pDataPtr);

		return dynamic_cast<IBuffer*>(pGraphicsContextInstance);
	}
}

#endif