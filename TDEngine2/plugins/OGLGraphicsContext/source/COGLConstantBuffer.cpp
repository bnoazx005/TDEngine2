#include "./../include/COGLConstantBuffer.h"
#include "./../include/COGLBuffer.h"
#include <core/IGraphicsContext.h>
#include <memory>


namespace TDEngine2
{
	COGLConstantBuffer::COGLConstantBuffer() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE COGLConstantBuffer::Init(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, U32 totalBufferSize, void* pDataPtr)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		mpBufferImpl = CreateOGLBuffer(pGraphicsContext, usageType, COGLBuffer::BT_CONSTANT_BUFFER, totalBufferSize, pDataPtr, result);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE COGLConstantBuffer::Free()
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

	E_RESULT_CODE COGLConstantBuffer::Map(E_BUFFER_MAP_TYPE mapType)
	{
		return mpBufferImpl->Map(mapType);
	}

	void COGLConstantBuffer::Unmap()
	{
		return mpBufferImpl->Unmap();
	}

	E_RESULT_CODE COGLConstantBuffer::Write(const void* pData, U32 size)
	{
		return mpBufferImpl->Write(pData, size);
	}

	void* COGLConstantBuffer::Read()
	{
		return mpBufferImpl->Read();
	}

	const TBufferInternalData& COGLConstantBuffer::GetInternalData() const
	{
		return mpBufferImpl->GetInternalData();
	}

	U32 COGLConstantBuffer::GetSize() const
	{
		return mpBufferImpl->GetSize();
	}


	TDE2_API IConstantBuffer* CreateOGLConstantBuffer(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType,
													  U32 totalBufferSize, void* pDataPtr, E_RESULT_CODE& result)
	{
		COGLConstantBuffer* pConstantBufferInstance = new (std::nothrow) COGLConstantBuffer();

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