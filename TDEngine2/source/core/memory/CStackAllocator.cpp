#include "../../../include/core/memory/CStackAllocator.h"
#include "../../../include/editor/CMemoryProfiler.h"
#include <cstring>


namespace TDEngine2
{
	U8 CStackAllocator::mHeaderSize = sizeof(CStackAllocator::TAllocHeader);


	CStackAllocator::CStackAllocator() :
		CBaseAllocator()
	{
	}

	void* CStackAllocator::Allocate(TSizeType size, U8 alignment)
	{
		const U8 padding = CBaseAllocator::GetPaddingWithHeader(_getCurrFitBlock(size)->mpCurrPointer, alignment, mHeaderSize);

		auto pCurrBlock = _getCurrFitBlock(size + padding); /// \note Check the second time according to computed padding

		U32Ptr alignedAddress = reinterpret_cast<U32Ptr>(pCurrBlock->mpCurrPointer) + padding;

		TAllocHeaderPtr pAllocHeader = (TAllocHeaderPtr)(alignedAddress - mHeaderSize);

		pAllocHeader->mPadding = padding;

		pCurrBlock->mpCurrPointer = reinterpret_cast<void*>(alignedAddress + size);
		pCurrBlock->mUsedMemorySize += (padding + size);

		TDE2_UPDATE_MEMORY_BLOCK_INFO(mName, GetUsedMemorySize());

		++mAllocationsCount;

		return reinterpret_cast<void*>(alignedAddress);
	}

	E_RESULT_CODE CStackAllocator::Deallocate(void* pObjectPtr)
	{
		if (!pObjectPtr)
		{
			return RC_INVALID_ARGS;
		}

		auto pOwningMemoryRegion = _findOwnerBlock(pObjectPtr);
		if (!pOwningMemoryRegion)
		{
			TDE2_ASSERT(false);
			return RC_FAIL;
		}

		TAllocHeaderPtr allocHeader = (TAllocHeaderPtr)(reinterpret_cast<U32Ptr>(pObjectPtr) - static_cast<U32Ptr>(mHeaderSize));

		pOwningMemoryRegion->mUsedMemorySize -= allocHeader->mPadding + (reinterpret_cast<U32Ptr>(pOwningMemoryRegion->mpCurrPointer) - reinterpret_cast<U32Ptr>(pObjectPtr));
		pOwningMemoryRegion->mpCurrPointer = reinterpret_cast<void*>(reinterpret_cast<U32Ptr>(pObjectPtr) - static_cast<U32Ptr>(allocHeader->mPadding));

		--mAllocationsCount;

		return RC_OK;
	}

	E_RESULT_CODE CStackAllocator::Clear()
	{
		return RC_FAIL;
	}


	TDE2_API IAllocator* CreateStackAllocator(USIZE pageSize, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAllocator, CStackAllocator, result, pageSize);
	}
}