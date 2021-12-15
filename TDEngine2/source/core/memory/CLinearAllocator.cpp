#include "../../../include/core/memory/CLinearAllocator.h"
#include "../../../include/editor/CMemoryProfiler.h"
#include "../../../include/utils/CFileLogger.h"
#include "stringUtils.hpp"
#include <cstring>


namespace TDEngine2
{
	CLinearAllocator::CLinearAllocator():
		CBaseAllocator(), mpCurrPos(nullptr)
	{
	}

	E_RESULT_CODE CLinearAllocator::Init(TSizeType totalMemorySize, U8* pMemoryBlock)
	{
		E_RESULT_CODE result = CBaseAllocator::Init(totalMemorySize, pMemoryBlock);

		if (result != RC_OK)
		{
			return result;
		}

		mpCurrPos = mpMemoryBlock;

		return RC_OK;
	}

	void* CLinearAllocator::Allocate(TSizeType size, U8 alignment)
	{
		U8 padding = CBaseAllocator::GetPadding(mpCurrPos, alignment);

		if (mUsedMemorySize + padding + size > mTotalMemorySize)
		{
			TDE2_ASSERT(false);
			return nullptr;
		}

		U32Ptr alignedAddress = reinterpret_cast<U32Ptr>(mpCurrPos) + padding;

		mpCurrPos = reinterpret_cast<void*>(alignedAddress + size);

		mUsedMemorySize += (padding + size);

		TDE2_UPDATE_MEMORY_BLOCK_INFO(mName, mUsedMemorySize);

		++mAllocationsCount;

		//LOG_MESSAGE(Wrench::StringUtils::Format("alloc count: {0}; used memory : {1}, stashed memory : {2}", mAllocationsCount, mUsedMemorySize, mTotalMemorySize - mUsedMemorySize));
		
		return reinterpret_cast<void*>(alignedAddress);
	}

	E_RESULT_CODE CLinearAllocator::Deallocate(void* pObjectPtr)
	{
		return RC_OK;
	}

	E_RESULT_CODE CLinearAllocator::Clear()
	{
		mUsedMemorySize = 0;

		mAllocationsCount = 0;

		mpCurrPos = mpMemoryBlock;

		return RC_OK;
	}


	TDE2_API IAllocator* CreateLinearAllocator(USIZE totalMemorySize, U8* pMemoryBlock, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAllocator, CLinearAllocator, result, totalMemorySize, pMemoryBlock);
	}
}