#include "../../../include/core/memory/CBaseAllocator.h"
#include <cstring>


namespace TDEngine2
{
	TBaseAllocatorParams::TBaseAllocatorParams(USIZE size):
		mMemoryBlockSize(size)
	{
	}

	CBaseAllocator::CBaseAllocator():
		CBaseObject(), 
		mAllocationsCount(0)
	{
	}

	E_RESULT_CODE CBaseAllocator::Init(TSizeType pageSize)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pageSize)
		{
			return RC_INVALID_ARGS;
		}

		mPageSize = pageSize;

		mpRootBlock = std::make_unique<TMemoryBlockEntity>();

		mpRootBlock->mpRegion        = std::make_unique<U8[]>(mPageSize);
		mpRootBlock->mpCurrPointer   = reinterpret_cast<void*>(mpRootBlock->mpRegion.get());
		mpRootBlock->mpNextBlock     = nullptr;
		mpRootBlock->mUsedMemorySize = 0;

		mIsInitialized = true;

		return RC_OK;
	}

#if TDE2_EDITORS_ENABLED

	void CBaseAllocator::SetBlockDebugName(const std::string& blockId)
	{
		mName = blockId;
	}

#endif

	CBaseAllocator::TSizeType CBaseAllocator::GetTotalMemorySize() const
	{
		if (!mpRootBlock)
		{
			return 0;
		}

		const TMemoryBlockEntity* pCurrBlock = mpRootBlock.get();

		U32 totalMemorySize = 1;

		while (pCurrBlock->mpNextBlock)
		{
			pCurrBlock = pCurrBlock->mpNextBlock.get();
			++totalMemorySize;
		} 

		return totalMemorySize * mPageSize;
	}

	CBaseAllocator::TSizeType CBaseAllocator::GetUsedMemorySize() const
	{
		if (!mpRootBlock)
		{
			return 0;
		}

		const TMemoryBlockEntity* pCurrBlock = mpRootBlock.get();

		TSizeType usedMemorySize = 0;

		while (pCurrBlock)
		{
			usedMemorySize += pCurrBlock->mUsedMemorySize;
			pCurrBlock = pCurrBlock->mpNextBlock.get();
		}

		return usedMemorySize;
	}

	U32 CBaseAllocator::GetAllocationsCount() const
	{
		return mAllocationsCount;
	}

	TDE2_API void* CBaseAllocator::GetAlignedAddress(void* pAddress, U8 alignment)
	{
		TDE2_ASSERT(alignment);
		return reinterpret_cast<void*>((reinterpret_cast<U32Ptr>(pAddress) + static_cast<U32Ptr>(alignment - 1)) & static_cast<U32Ptr>(~(alignment - 1)));
	}

	TDE2_API U8 CBaseAllocator::GetPadding(void* pAddress, U8 alignment)
	{
		TDE2_ASSERT(alignment);

		U8 padding = alignment - (reinterpret_cast<U32Ptr>(pAddress) & static_cast<U32Ptr>(alignment - 1));

		return padding == alignment ? 0 : padding;
	}

	TDE2_API U8 CBaseAllocator::GetPaddingWithHeader(void* pAddress, U8 alignment, U8 headerSize)
	{
		U8 padding = GetPadding(pAddress, alignment);

		if (padding < headerSize)
		{
			headerSize -= padding;
			
			padding += alignment * (headerSize / alignment);

			if (headerSize % alignment > 0)
			{
				padding += alignment;
			}
		}

		return padding;
	}

	CBaseAllocator::TMemoryBlockEntity* CBaseAllocator::_getCurrFitBlock(TSizeType allocationSize)
	{
		auto pCurrBlock = mpRootBlock.get();
		auto pPrevBlock = pCurrBlock;

		while (pCurrBlock)
		{
			if (pCurrBlock->mUsedMemorySize + allocationSize <= mPageSize)
			{
				break;
			}

			pPrevBlock = pCurrBlock;
			pCurrBlock = pCurrBlock->mpNextBlock ? pCurrBlock->mpNextBlock.get() : nullptr;
		}

		if (!pCurrBlock) /// \note Create a new block because all are filled up or there are no ones
		{
			return _allocateNewBlock(pPrevBlock);
		}

		return pCurrBlock;
	}

	CBaseAllocator::TMemoryBlockEntity* CBaseAllocator::_allocateNewBlock(TMemoryBlockEntity* pPrevBlockEntity)
	{
		auto pNewBlockEntity = std::make_unique<TMemoryBlockEntity>();

		pNewBlockEntity->mpRegion        = std::make_unique<U8[]>(mPageSize);
		pNewBlockEntity->mpCurrPointer   = reinterpret_cast<void*>(pNewBlockEntity->mpRegion.get());
		pNewBlockEntity->mUsedMemorySize = 0;
		pNewBlockEntity->mpNextBlock     = nullptr;

		pPrevBlockEntity->mpNextBlock = std::move(pNewBlockEntity);

		return pPrevBlockEntity->mpNextBlock.get();
	}

	CBaseAllocator::TMemoryBlockEntity* CBaseAllocator::_findOwnerBlock(void* pObjectPtr)
	{
		TMemoryBlockEntity* pCurrBlock = mpRootBlock.get();

		while (pCurrBlock)
		{
			const U32Ptr regionBegin = reinterpret_cast<U32Ptr>(pCurrBlock->mpRegion.get());
			const U32Ptr regionEnd   = regionBegin + mPageSize;

			const U32Ptr objectPtr = reinterpret_cast<U32Ptr>(pObjectPtr);

			if (objectPtr >= regionBegin && objectPtr < regionEnd)
			{
				return pCurrBlock;
			}

			pCurrBlock = pCurrBlock->mpNextBlock ? pCurrBlock->mpNextBlock.get() : nullptr;
		}

		return nullptr;
	}

	CBaseAllocator::TMemoryBlockEntity* CBaseAllocator::_getLastBlockEntity() const
	{
		TMemoryBlockEntity* pCurrBlock = mpRootBlock.get();

		while (pCurrBlock->mpNextBlock)
		{
			pCurrBlock = pCurrBlock->mpNextBlock.get();
		}

		return pCurrBlock;
	}

	bool CBaseAllocator::_isAllocationPossible(TSizeType allocationSize) const
	{
		return allocationSize <= mPageSize;
	}

	TDE2_API void* AllocateMemory(IAllocator* pAllocator, USIZE size, USIZE alignment)
	{
		if (!pAllocator)
		{
			return nullptr;
		}

		return pAllocator->Allocate(size, static_cast<U8>(alignment));
	}
}