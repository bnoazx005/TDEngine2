#include "./../../../include/core/memory/CLinearAllocator.h"
#include "../../../include/editor/CMemoryProfiler.h"
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

		U32Ptr alignedAddress = reinterpret_cast<U32Ptr>(mpCurrPos) + padding;

		mpCurrPos = reinterpret_cast<void*>(alignedAddress + size);

		mUsedMemorySize += (padding + size);

		TDE2_UPDATE_MEMORY_BLOCK_INFO(mName, mUsedMemorySize);

		++mAllocationsCount;

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


	TDE2_API IAllocator* CreateLinearAllocator(U32 totalMemorySize, U8* pMemoryBlock, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAllocator, CLinearAllocator, result, totalMemorySize, pMemoryBlock);
	}


	CLinearAllocatorFactory::CLinearAllocatorFactory():
		CBaseAllocatorFactory()
	{
	}

	TResult<IAllocator*> CLinearAllocatorFactory::Create(U8* pMemoryBlock, const TBaseAllocatorParams& params) const
	{
		if (!pMemoryBlock)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		E_RESULT_CODE result = RC_OK;

		IAllocator* pAllocator = CreateLinearAllocator(params.mMemoryBlockSize, pMemoryBlock, result);

		if (result != RC_OK)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return Wrench::TOkValue<IAllocator*>(pAllocator);
	}

	TypeId CLinearAllocatorFactory::GetAllocatorType() const
	{
		return CLinearAllocator::GetTypeId();
	}


	TDE2_API IAllocatorFactory* CreateLinearAllocatorFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAllocatorFactory, CLinearAllocatorFactory, result);
	}
}