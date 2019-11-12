#include "./../include/COGLIndexBuffer.h"
#include "./../include/COGLBuffer.h"
#include "./../include/COGLUtils.h"
#include <core/IGraphicsContext.h>
#include <memory>


namespace TDEngine2
{
	COGLIndexBuffer::COGLIndexBuffer() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE COGLIndexBuffer::Init(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, U32 totalBufferSize,
										E_INDEX_FORMAT_TYPE indexFormatType, const void* pDataPtr)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIndexFormatType = indexFormatType;

		E_RESULT_CODE result = RC_OK;

		mpBufferImpl = CreateOGLBuffer(pGraphicsContext, usageType, COGLBuffer::BT_INDEX_BUFFER, totalBufferSize, pDataPtr, result);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE COGLIndexBuffer::Free()
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

	E_RESULT_CODE COGLIndexBuffer::Map(E_BUFFER_MAP_TYPE mapType)
	{
		return mpBufferImpl->Map(mapType);
	}

	void COGLIndexBuffer::Unmap()
	{
		return mpBufferImpl->Unmap();
	}

	E_RESULT_CODE COGLIndexBuffer::Write(const void* pData, U32 size)
	{
		return mpBufferImpl->Write(pData, size);
	}

	void* COGLIndexBuffer::Read()
	{
		return mpBufferImpl->Read();
	}

	void COGLIndexBuffer::Bind(U32 offset)
	{
		GL_SAFE_VOID_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mpBufferImpl->GetInternalData().mGLBuffer));
	}

	const TBufferInternalData& COGLIndexBuffer::GetInternalData() const
	{
		return mpBufferImpl->GetInternalData();
	}

	U32 COGLIndexBuffer::GetSize() const
	{
		return mpBufferImpl->GetSize();
	}

	E_INDEX_FORMAT_TYPE COGLIndexBuffer::GetIndexFormat() const
	{
		return mIndexFormatType;
	}


	TDE2_API IIndexBuffer* CreateOGLIndexBuffer(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, E_INDEX_FORMAT_TYPE indexFormatType,
		U32 totalBufferSize, const void* pDataPtr, E_RESULT_CODE& result)
	{
		COGLIndexBuffer* pIndexBufferInstance = new (std::nothrow) COGLIndexBuffer();

		if (!pIndexBufferInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pIndexBufferInstance->Init(pGraphicsContext, usageType, totalBufferSize, indexFormatType, pDataPtr);

		if (result != RC_OK)
		{
			delete pIndexBufferInstance;

			pIndexBufferInstance = nullptr;
		}

		return dynamic_cast<IIndexBuffer*>(pIndexBufferInstance);
	}
}