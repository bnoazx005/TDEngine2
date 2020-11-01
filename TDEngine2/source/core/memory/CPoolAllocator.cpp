#include "./../../../include/core/memory/CPoolAllocator.h"


namespace TDEngine2
{
	TPoolAllocatorParams::TPoolAllocatorParams(U32 size, U32 perElementSize, U32 elementAlignment):
		TBaseAllocatorParams(size), mPerObjectSize(perElementSize), mObjectAlignment(elementAlignment)
	{
	}


	CPoolAllocator::CPoolAllocator():
		CBaseAllocator(), mObjectSize(0), mObjectAlignment(0),
		mppNextFreeBlock(nullptr)
	{
	}

	E_RESULT_CODE CPoolAllocator::Init(U32 objectSize, U32 objectAlignment, U32 totalMemorySize, U8* pMemoryBlock)
	{
		E_RESULT_CODE result = CBaseAllocator::Init(totalMemorySize, pMemoryBlock);

		if (result != RC_OK)
		{
			return result;
		}

		mObjectSize = objectSize;

		mObjectAlignment = objectAlignment;

		return Clear();
	}

	void* CPoolAllocator::Allocate(U32 size, U8 alignment)
	{
		if (!mppNextFreeBlock)
		{
			return nullptr;
		}

		void* pObjectPtr = mppNextFreeBlock;

		mppNextFreeBlock = reinterpret_cast<void**>(*mppNextFreeBlock);

		mUsedMemorySize += mObjectSize;

		++mAllocationsCount;

		return pObjectPtr;
	}

	E_RESULT_CODE CPoolAllocator::Deallocate(void* pObjectPtr)
	{
		if (!pObjectPtr)
		{
			return RC_INVALID_ARGS;
		}

		*(reinterpret_cast<void**>(pObjectPtr)) = mppNextFreeBlock;

		mppNextFreeBlock = reinterpret_cast<void**>(pObjectPtr);

		mUsedMemorySize -= mObjectSize;

		--mAllocationsCount;

		return RC_OK;
	}

	E_RESULT_CODE CPoolAllocator::Clear()
	{
		U8 padding = CBaseAllocator::GetPadding(mpMemoryBlock, mObjectAlignment);

		mppNextFreeBlock = reinterpret_cast<void**>(reinterpret_cast<U32Ptr>(mpMemoryBlock) + static_cast<U32Ptr>(mObjectAlignment));

		U32 numOfObjects = (mTotalMemorySize - padding) / mObjectSize;

		void** pCurrBlock = mppNextFreeBlock;

		for (U32 i = 0; i < numOfObjects - 1; ++i)
		{
			*pCurrBlock = reinterpret_cast<void*>(reinterpret_cast<U32Ptr>(pCurrBlock) + mObjectSize);

			pCurrBlock = reinterpret_cast<void**>(*pCurrBlock);
		}

		return RC_OK;
	}


	TDE2_API IAllocator* CreatePoolAllocator(U32 objectSize, U32 objectAlignment, U32 totalMemorySize, U8* pMemoryBlock, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAllocator, CPoolAllocator, result, objectSize, objectAlignment, totalMemorySize, pMemoryBlock);
	}


	CPoolAllocatorFactory::CPoolAllocatorFactory() :
		CBaseAllocatorFactory()
	{
	}

	TResult<IAllocator*> CPoolAllocatorFactory::Create(U8* pMemoryBlock, const TBaseAllocatorParams& params) const
	{
		const TPoolAllocatorParams* pPoolParams = dynamic_cast<const TPoolAllocatorParams*>(&params);

		if (!pPoolParams || !pMemoryBlock)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		E_RESULT_CODE result = RC_OK;

		IAllocator* pAllocator = CreatePoolAllocator(pPoolParams->mPerObjectSize, pPoolParams->mObjectAlignment, 
													 pPoolParams->mMemoryBlockSize, pMemoryBlock, result);

		if (result != RC_OK)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return Wrench::TOkValue<IAllocator*>(pAllocator);
	}

	TypeId CPoolAllocatorFactory::GetAllocatorType() const
	{
		return CPoolAllocator::GetTypeId();
	}


	TDE2_API IAllocatorFactory* CreatePoolAllocatorFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAllocatorFactory, CPoolAllocatorFactory, result);
	}
}