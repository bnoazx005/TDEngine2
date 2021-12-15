#include "../../../include/core/memory/CPoolAllocator.h"
#include "../../../include/editor/CMemoryProfiler.h"


namespace TDEngine2
{
	TPoolAllocatorParams::TPoolAllocatorParams(USIZE size, USIZE perElementSize, USIZE elementAlignment):
		TBaseAllocatorParams(size), mPerObjectSize(perElementSize), mObjectAlignment(elementAlignment)
	{
	}


	CPoolAllocator::CPoolAllocator():
		CBaseAllocator(), mObjectSize(0), mObjectAlignment(0),
		mppNextFreeBlock(nullptr)
	{
	}

	E_RESULT_CODE CPoolAllocator::Init(TSizeType objectSize, TSizeType objectAlignment, TSizeType totalMemorySize, U8* pMemoryBlock)
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

	void* CPoolAllocator::Allocate(TSizeType size, U8 alignment)
	{
		if (!mppNextFreeBlock)
		{
			return nullptr;
		}

		void* pObjectPtr = mppNextFreeBlock;

		mppNextFreeBlock = reinterpret_cast<void**>(*mppNextFreeBlock);

		mUsedMemorySize += mObjectSize;

		++mAllocationsCount;

		TDE2_UPDATE_MEMORY_BLOCK_INFO(mName, mUsedMemorySize);

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
		U8 padding = CBaseAllocator::GetPadding(mpMemoryBlock, static_cast<U8>(mObjectAlignment));

		mppNextFreeBlock = reinterpret_cast<void**>(reinterpret_cast<U32Ptr>(mpMemoryBlock) + static_cast<U32Ptr>(mObjectAlignment));

		U32 numOfObjects = static_cast<U32>((mTotalMemorySize - padding) / mObjectSize);

		void** pCurrBlock = mppNextFreeBlock;

		for (U32 i = 0; i < numOfObjects - 1; ++i)
		{
			*pCurrBlock = reinterpret_cast<void*>(reinterpret_cast<U32Ptr>(pCurrBlock) + mObjectSize);

			pCurrBlock = reinterpret_cast<void**>(*pCurrBlock);
		}

		return RC_OK;
	}


	TDE2_API IAllocator* CreatePoolAllocator(USIZE objectSize, USIZE objectAlignment, USIZE totalMemorySize, U8* pMemoryBlock, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAllocator, CPoolAllocator, result, objectSize, objectAlignment, totalMemorySize, pMemoryBlock);
	}
}