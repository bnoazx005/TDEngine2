#include "./../include/COGLConstantBuffer.h"
#include "./../include/COGLBuffer.h"
#include "./../include/COGLUtils.h"
#include <core/IGraphicsContext.h>
#include <memory>


namespace TDEngine2
{
	COGLConstantBuffer::COGLConstantBuffer() :
		CBaseObject()
	{
	}

	E_RESULT_CODE COGLConstantBuffer::Init(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType, USIZE totalBufferSize, const void* pDataPtr)
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

	E_RESULT_CODE COGLConstantBuffer::_onFreeInternal()
	{
		return mpBufferImpl->Free();
	}

	E_RESULT_CODE COGLConstantBuffer::Map(E_BUFFER_MAP_TYPE mapType)
	{
		return mpBufferImpl->Map(mapType);
	}

	void COGLConstantBuffer::Unmap()
	{
		return mpBufferImpl->Unmap();
	}

	E_RESULT_CODE COGLConstantBuffer::Write(const void* pData, USIZE size)
	{
		return mpBufferImpl->Write(pData, size);
	}

	void* COGLConstantBuffer::Read()
	{
		return mpBufferImpl->Read();
	}

	void COGLConstantBuffer::Bind(U32 slot)
	{
		if (!mIsInitialized)
		{
			return;
		}

		mCurrUsedSlot = slot;

		GL_SAFE_VOID_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, slot, mpBufferImpl->GetInternalData().mGLBuffer));
	}

	void COGLConstantBuffer::Unbind()
	{
		if (!mIsInitialized && !mCurrUsedSlot)
		{
			return;
		}

		GL_SAFE_VOID_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, mCurrUsedSlot, 0));

		mCurrUsedSlot = 0;
	}

	const TBufferInternalData& COGLConstantBuffer::GetInternalData() const
	{
		return mpBufferImpl->GetInternalData();
	}

	USIZE COGLConstantBuffer::GetSize() const
	{
		return mpBufferImpl->GetSize();
	}

	USIZE COGLConstantBuffer::GetUsedSize() const
	{
		return mpBufferImpl->GetUsedSize();
	}


	TDE2_API IConstantBuffer* CreateOGLConstantBuffer(IGraphicsContext* pGraphicsContext, E_BUFFER_USAGE_TYPE usageType,
													  USIZE totalBufferSize, const void* pDataPtr, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IConstantBuffer, COGLConstantBuffer, result, pGraphicsContext, usageType, totalBufferSize, pDataPtr);
	}
}