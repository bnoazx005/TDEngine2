#include "../../../include/core/memory/CLinearAllocator.h"
#include "../../../include/editor/CMemoryProfiler.h"
#include "../../../include/editor/CPerfProfiler.h"
#include "../../../include/utils/CFileLogger.h"
#include "stringUtils.hpp"
#include <cstring>


namespace TDEngine2
{
	CLinearAllocator::CLinearAllocator():
		CBaseAllocator()
	{
	}

	void* CLinearAllocator::Allocate(TSizeType size, U8 alignment)
	{
		if (!_isAllocationPossible(size))
		{
			 /// \note Can't allocate contiguous block of size greater than our pages
			return nullptr;
		}

		const U8 padding = CBaseAllocator::GetPadding(_getCurrFitBlock(size)->mpCurrPointer, alignment);

		auto pCurrBlock = _getCurrFitBlock(size + padding); /// \note Check the second time according to computed padding

		U32Ptr alignedAddress = reinterpret_cast<U32Ptr>(pCurrBlock->mpCurrPointer) + padding;
		TDE2_ASSERT(alignedAddress);

		pCurrBlock->mpCurrPointer = reinterpret_cast<void*>(alignedAddress + size);
		pCurrBlock->mUsedMemorySize += (padding + size);

		TDE2_UPDATE_MEMORY_BLOCK_INFO(mName, GetUsedMemorySize());
		++mAllocationsCount;

		return reinterpret_cast<void*>(alignedAddress);
	}

	E_RESULT_CODE CLinearAllocator::Deallocate(void* pObjectPtr)
	{
		return RC_OK;
	}

	E_RESULT_CODE CLinearAllocator::Clear()
	{
		TDE2_PROFILER_SCOPE("CLinearAllocator::Clear");
		mAllocationsCount = 0;

		TMemoryBlockEntity* pCurrBlock = mpRootBlock.get();

		while (pCurrBlock)
		{
			pCurrBlock->mUsedMemorySize = 0;
			pCurrBlock->mpCurrPointer = reinterpret_cast<void*>(pCurrBlock->mpRegion.get());

			pCurrBlock = pCurrBlock->mpNextBlock ? pCurrBlock->mpNextBlock.get() : nullptr;
		}

		return RC_OK;
	}


	TDE2_API IAllocator* CreateLinearAllocator(USIZE pageSize, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAllocator, CLinearAllocator, result, pageSize);
	}
}