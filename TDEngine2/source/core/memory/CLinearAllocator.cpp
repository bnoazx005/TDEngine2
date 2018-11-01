#include "./../../../include/core/memory/CLinearAllocator.h"
#include <cstring>


namespace TDEngine2
{
	CLinearAllocator::CLinearAllocator():
		CBaseAllocator(), mpCurrPos(nullptr)
	{
	}

	E_RESULT_CODE CLinearAllocator::Init(U32 totalMemorySize, U8* pMemoryBlock)
	{
		E_RESULT_CODE result = CBaseAllocator::Init(totalMemorySize, pMemoryBlock);

		if (result != RC_OK)
		{
			return result;
		}

		mpCurrPos = mpMemoryBlock;

		return RC_OK;
	}

	void* CLinearAllocator::Allocate(U32 size, U8 alignment)
	{
		U8 padding = CBaseAllocator::GetPadding(mpCurrPos, alignment);

		if (mUsedMemorySize + padding + size > mTotalMemorySize)
		{
			return nullptr;
		}

		U32 alignedAddress = reinterpret_cast<U32Ptr>(mpCurrPos) + padding;

		mpCurrPos = reinterpret_cast<void*>(alignedAddress + size);

		mUsedMemorySize += (padding + size);

		++mAllocationsCount;

		return reinterpret_cast<void*>(alignedAddress);
	}

	E_RESULT_CODE CLinearAllocator::Deallocate(void* pObjectPtr)
	{
		return RC_FAIL;
	}

	E_RESULT_CODE CLinearAllocator::Clear()
	{
		mUsedMemorySize = 0;

		mAllocationsCount = 0;

		mpCurrPos = mpMemoryBlock;

		return RC_OK;
	}


	TDE2_API IAllocator* CreateLinearAllocator(U32 totalMemorySize, U8* pMemoryBlock, E_RESULT_CODE& result)
	{
		CLinearAllocator* pLinearAllocatorInstance = new (std::nothrow) CLinearAllocator();

		if (!pLinearAllocatorInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pLinearAllocatorInstance->Init(totalMemorySize, pMemoryBlock);

		if (result != RC_OK)
		{
			delete pLinearAllocatorInstance;

			pLinearAllocatorInstance = nullptr;
		}

		return dynamic_cast<IAllocator*>(pLinearAllocatorInstance);
	}
}