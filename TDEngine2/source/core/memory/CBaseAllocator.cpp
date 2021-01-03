#include "./../../../include/core/memory/CBaseAllocator.h"
#include <cstring>


namespace TDEngine2
{
	TBaseAllocatorParams::TBaseAllocatorParams(U32 size):
		mMemoryBlockSize(size)
	{
	}

	CBaseAllocator::CBaseAllocator():
		CBaseObject(), mTotalMemorySize(0), mUsedMemorySize(0),
		mAllocationsCount(0), mpMemoryBlock(nullptr)
	{
	}

	E_RESULT_CODE CBaseAllocator::Init(U32 totalMemorySize, U8* pMemoryBlock)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pMemoryBlock)
		{
			return RC_INVALID_ARGS;
		}

		mTotalMemorySize = totalMemorySize;

		mpMemoryBlock = pMemoryBlock;

		memset(mpMemoryBlock, 0, mTotalMemorySize);

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBaseAllocator::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

#if TDE2_EDITORS_ENABLED

	void CBaseAllocator::SetBlockDebugName(const std::string& blockId)
	{
		mName = blockId;
	}

#endif

	U32 CBaseAllocator::GetTotalMemorySize() const
	{
		return mTotalMemorySize;
	}

	U32 CBaseAllocator::GetUsedMemorySize() const
	{
		return mUsedMemorySize;
	}

	U32 CBaseAllocator::GetAllocationsCount() const
	{
		return mAllocationsCount;
	}

	TDE2_API void* CBaseAllocator::GetAlignedAddress(void* pAddress, U8 alignment)
	{
		return reinterpret_cast<void*>((reinterpret_cast<U32Ptr>(pAddress) + static_cast<U32Ptr>(alignment - 1)) & static_cast<U32Ptr>(~(alignment - 1)));
	}

	TDE2_API U8 CBaseAllocator::GetPadding(void* pAddress, U8 alignment)
	{
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


	TDE2_API void* AllocateMemory(IAllocator* pAllocator, U32 size, U32 alignment)
	{
		if (!pAllocator)
		{
			return nullptr;
		}

		return pAllocator->Allocate(size, alignment);
	}


	CBaseAllocatorFactory::CBaseAllocatorFactory():
		CBaseObject()
	{
	}

	E_RESULT_CODE CBaseAllocatorFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBaseAllocatorFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}
}