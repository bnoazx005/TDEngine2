#include "../../../include/core/memory/CPoolAllocator.h"
#include "../../../include/editor/CMemoryProfiler.h"
#include "../../../include/editor/CPerfProfiler.h"


namespace TDEngine2
{
	TPoolAllocatorParams::TPoolAllocatorParams(USIZE size, USIZE perElementSize, USIZE elementAlignment):
		TBaseAllocatorParams(size), mPerObjectSize(perElementSize), mObjectAlignment(elementAlignment)
	{
	}


	CPoolAllocator::CPoolAllocator():
		CBaseAllocator(), mObjectSize(0), mObjectAlignment(0),
		mppNextFreeBlock(nullptr), mUsedMemorySize(0)
	{
	}

	E_RESULT_CODE CPoolAllocator::Init(TSizeType objectSize, TSizeType objectAlignment, TSizeType pageSize)
	{
		E_RESULT_CODE result = CBaseAllocator::Init(pageSize);
		if (result != RC_OK)
		{
			return result;
		}

		mObjectAlignment = objectAlignment;
		mObjectSize      = objectSize;

		return Clear();
	}

	void* CPoolAllocator::Allocate(TSizeType size, U8 alignment)
	{
		if (size != mObjectSize || !_isAllocationPossible(size))
		{
			return nullptr;
		}

		if (!mppNextFreeBlock)
		{
			auto pLastCreatedBlock = _getLastBlockEntity();
			auto pNewBlock = _allocateNewBlock(pLastCreatedBlock);

			_clearMemoryRegion(pNewBlock);

			/// \note Stitch both blocks together
			*pLastCreatedBlock->mpLastAllowedPointer = pNewBlock->mpCurrPointer;

			mppNextFreeBlock = reinterpret_cast<void**>(reinterpret_cast<U32Ptr>(pNewBlock->mpRegion.get()) + static_cast<U32Ptr>(mObjectAlignment));
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
		TDE2_PROFILER_SCOPE("CPoolAllocator::Clear");

		TMemoryBlockEntity* pCurrBlockEntity = mpRootBlock.get();

		while (pCurrBlockEntity)
		{
			_clearMemoryRegion(pCurrBlockEntity);

			pCurrBlockEntity = pCurrBlockEntity->mpNextBlock ? pCurrBlockEntity->mpNextBlock.get() : nullptr;
		}

		/// \note Stitch all memory regions together
		{
			pCurrBlockEntity = mpRootBlock.get();

			while (pCurrBlockEntity)
			{
				TMemoryBlockEntity* pNextBlockEntity = pCurrBlockEntity->mpNextBlock ? pCurrBlockEntity->mpNextBlock.get() : nullptr;
				if (!pNextBlockEntity)
				{
					break;
				}

				*pCurrBlockEntity->mpLastAllowedPointer = pNextBlockEntity->mpCurrPointer;
				pCurrBlockEntity = pNextBlockEntity;
			}
		}

		mppNextFreeBlock = reinterpret_cast<void**>(reinterpret_cast<U32Ptr>(mpRootBlock->mpRegion.get()) + static_cast<U32Ptr>(mObjectAlignment));
		mUsedMemorySize = 0;

		return RC_OK;
	}

	CPoolAllocator::TSizeType CPoolAllocator::GetUsedMemorySize() const
	{
		return mUsedMemorySize;
	}

	void CPoolAllocator::_clearMemoryRegion(TMemoryBlockEntity*& pRegion)
	{
		void* pMemoryBlock = reinterpret_cast<void*>(pRegion->mpRegion.get());

		const U8 padding = CBaseAllocator::GetPadding(pMemoryBlock, static_cast<U8>(mObjectAlignment));

		void** pCurrBlock = reinterpret_cast<void**>(reinterpret_cast<U32Ptr>(pMemoryBlock) + static_cast<U32Ptr>(mObjectAlignment));

		pRegion->mpCurrPointer = pCurrBlock;

		for (U32 i = 0; i < static_cast<U32>((mPageSize - padding) / mObjectSize) - 1; ++i)
		{
			*pCurrBlock = reinterpret_cast<void*>(reinterpret_cast<U32Ptr>(pCurrBlock) + mObjectSize);
			pCurrBlock = reinterpret_cast<void**>(*pCurrBlock);
		}

		pRegion->mpLastAllowedPointer = pCurrBlock;
		pRegion->mUsedMemorySize = 0;

		*pCurrBlock = nullptr;
	}


	TDE2_API IAllocator* CreatePoolAllocator(USIZE objectSize, USIZE objectAlignment, USIZE pageSize, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAllocator, CPoolAllocator, result, objectSize, objectAlignment, pageSize);
	}


	/*!
		\brief CPoolAllocatorsRegistry's definition
	*/

	static std::vector<TPtr<IAllocator>> TypesPoolAllocators;

	IAllocator* CPoolAllocatorsRegistry::GetAllocator(USIZE objectSize, USIZE objectAlignment, USIZE pageSize)
	{
		E_RESULT_CODE result = RC_OK;
		TypesPoolAllocators.push_back(TPtr<IAllocator>(CreatePoolAllocator(objectSize, objectAlignment, pageSize, result)));
		TDE2_ASSERT(RC_OK == result);

		return TypesPoolAllocators.back().Get();
	}

	void CPoolAllocatorsRegistry::ClearAllAllocators()
	{
		TypesPoolAllocators.clear();
	}
}