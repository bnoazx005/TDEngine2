#include "./../../include/graphics/CRenderQueue.h"


namespace TDEngine2
{
	CRenderQueue::CRenderQueue():
		CBaseObject()
	{
	}

	E_RESULT_CODE CRenderQueue::Init(IAllocator* pTempAllocator)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pTempAllocator)
		{
			return RC_INVALID_ARGS;
		}

		mpTempAllocator = pTempAllocator;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CRenderQueue::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}


	TDE2_API CRenderQueue* CreateRenderQueue(IAllocator* pTempAllocator, E_RESULT_CODE& result)
	{
		CRenderQueue* pRenderQueueInstance = new (std::nothrow) CRenderQueue();

		if (!pRenderQueueInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pRenderQueueInstance->Init(pTempAllocator);

		if (result != RC_OK)
		{
			delete pRenderQueueInstance;

			pRenderQueueInstance = nullptr;
		}

		return pRenderQueueInstance;
	}
}