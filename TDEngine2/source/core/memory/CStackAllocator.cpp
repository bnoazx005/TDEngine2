#include "../../../include/core/memory/CStackAllocator.h"
#include "../../../include/editor/CMemoryProfiler.h"
#include <cstring>


namespace TDEngine2
{
	U8 CStackAllocator::mHeaderSize = sizeof(CStackAllocator::TAllocHeader);


	CStackAllocator::CStackAllocator() :
		CBaseAllocator(), mpCurrPos(nullptr)
	{
	}

	E_RESULT_CODE CStackAllocator::Init(TSizeType totalMemorySize, U8* pMemoryBlock)
	{
		E_RESULT_CODE result = CBaseAllocator::Init(totalMemorySize, pMemoryBlock);

		if (result != RC_OK)
		{
			return result;
		}

		mpCurrPos = mpMemoryBlock;
		
		return RC_OK;
	}

	void* CStackAllocator::Allocate(TSizeType size, U8 alignment)
	{
		U8 padding = CBaseAllocator::GetPaddingWithHeader(mpCurrPos, alignment, mHeaderSize);

		if (mUsedMemorySize + padding + size > mTotalMemorySize)
		{
			return nullptr;
		}

		U32Ptr alignedAddress = reinterpret_cast<U32Ptr>(mpCurrPos) + padding;

		TAllocHeaderPtr pAllocHeader = (TAllocHeaderPtr)(alignedAddress - mHeaderSize);

		pAllocHeader->mPadding = padding;

		mpCurrPos = reinterpret_cast<void*>(alignedAddress + size);

		mUsedMemorySize += (padding + size);

		TDE2_UPDATE_MEMORY_BLOCK_INFO(mName, mUsedMemorySize);

		++mAllocationsCount;

		return reinterpret_cast<void*>(alignedAddress);
	}

	E_RESULT_CODE CStackAllocator::Deallocate(void* pObjectPtr)
	{
		if (!pObjectPtr)
		{
			return RC_INVALID_ARGS;
		}

		TAllocHeaderPtr allocHeader = (TAllocHeaderPtr)(reinterpret_cast<U32Ptr>(pObjectPtr) - static_cast<U32Ptr>(mHeaderSize));

		mUsedMemorySize -= allocHeader->mPadding + (reinterpret_cast<U32Ptr>(mpCurrPos) - reinterpret_cast<U32Ptr>(pObjectPtr));

		mpCurrPos = reinterpret_cast<void*>(reinterpret_cast<U32Ptr>(pObjectPtr) - static_cast<U32Ptr>(allocHeader->mPadding));

		--mAllocationsCount;

		return RC_OK;
	}

	E_RESULT_CODE CStackAllocator::Clear()
	{
		return RC_FAIL;
	}


	TDE2_API IAllocator* CreateStackAllocator(USIZE totalMemorySize, U8* pMemoryBlock, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAllocator, CStackAllocator, result, totalMemorySize, pMemoryBlock);
	}
}