#include "./../include/CD3D11VertexBuffer.h"
#include "./../include/CD3D11Buffer.h"
#include <core/IGraphicsContext.h>
#include <memory>


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	CD3D11VertexBuffer::CD3D11VertexBuffer() :
		mIsInitialized(false), mpInputLayout(nullptr)
	{
	}

	E_RESULT_CODE CD3D11VertexBuffer::Init(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, U32 totalBufferSize, void* pDataPtr)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}
		
		E_RESULT_CODE result = RC_OK;

		mpBufferImpl = CreateD3D11Buffer(pGraphicsContext, usageType, BT_VERTEX_BUFFER, totalBufferSize, pDataPtr, result);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D11VertexBuffer::Free()
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

	E_RESULT_CODE CD3D11VertexBuffer::Map(E_BUFFER_MAP_TYPE mapType)
	{
		return mpBufferImpl->Map(mapType);
	}

	void CD3D11VertexBuffer::Unmap()
	{
		return mpBufferImpl->Unmap();
	}

	E_RESULT_CODE CD3D11VertexBuffer::Write(const void* pData, U32 size)
	{
		return mpBufferImpl->Write(pData, size);
	}

	void* CD3D11VertexBuffer::Read()
	{
		return mpBufferImpl->Read();
	}

	void CD3D11VertexBuffer::SetInputLayout(ID3D11InputLayout* pInputLayout)
	{
		mpInputLayout = pInputLayout;
	}

	const TBufferInternalData& CD3D11VertexBuffer::GetInternalData() const
	{
		return mpBufferImpl->GetInternalData();
	}

	U32 CD3D11VertexBuffer::GetSize() const
	{
		return mpBufferImpl->GetSize();
	}


	TDE2_API IVertexBuffer* CreateD3D11VertexBuffer(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, 
													U32 totalBufferSize, void* pDataPtr, E_RESULT_CODE& result)
	{
		CD3D11VertexBuffer* pVertexBufferInstance = new (std::nothrow) CD3D11VertexBuffer();

		if (!pVertexBufferInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pVertexBufferInstance->Init(pGraphicsContext, usageType, totalBufferSize, pDataPtr);

		if (result != RC_OK)
		{
			delete pVertexBufferInstance;

			pVertexBufferInstance = nullptr;
		}

		return pVertexBufferInstance;
	}
}

#endif