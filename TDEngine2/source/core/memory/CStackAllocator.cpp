#include "./../../../include/core/memory/CStackAllocator.h"
#include <cstring>


namespace TDEngine2
{
	U8 CStackAllocator::mHeaderSize = sizeof(CStackAllocator::TAllocHeader);


	CStackAllocator::CStackAllocator() :
		CBaseAllocator(), mpCurrPos(nullptr)
	{
	}

	E_RESULT_CODE CStackAllocator::Init(U32 totalMemorySize, U8* pMemoryBlock)
	{
		E_RESULT_CODE result = CBaseAllocator::Init(totalMemorySize, pMemoryBlock);

		if (result != RC_OK)
		{
			return result;
		}

		mpCurrPos = mpMemoryBlock;
		
		return RC_OK;
	}

	void* CStackAllocator::Allocate(U32 size, U8 alignment)
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


	TDE2_API IAllocator* CreateStackAllocator(U32 totalMemorySize, U8* pMemoryBlock, E_RESULT_CODE& result)
	{
		CStackAllocator* pStackAllocatorInstance = new (std::nothrow) CStackAllocator();

		if (!pStackAllocatorInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pStackAllocatorInstance->Init(totalMemorySize, pMemoryBlock);

		if (result != RC_OK)
		{
			delete pStackAllocatorInstance;

			pStackAllocatorInstance = nullptr;
		}

		return dynamic_cast<IAllocator*>(pStackAllocatorInstance);
	}


	CStackAllocatorFactory::CStackAllocatorFactory() :
		CBaseAllocatorFactory()
	{
	}

	TResult<IAllocator*> CStackAllocatorFactory::Create(const TBaseAllocatorParams* pParams) const
	{
		if (!pParams)
		{
			return TErrorValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		E_RESULT_CODE result = RC_OK;

		IAllocator* pAllocator = CreateStackAllocator(pParams->mMemoryBlockSize, pParams->mpMemoryBlock, result);

		if (result != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		return TOkValue<IAllocator*>(pAllocator);
	}
	
	TypeId CStackAllocatorFactory::GetAllocatorType() const
	{
		return CStackAllocator::GetTypeId();
	}


	TDE2_API IAllocatorFactory* CreateStackAllocatorFactory(E_RESULT_CODE& result)
	{
		CStackAllocatorFactory* pStackAllocatorFactoryInstance = new (std::nothrow) CStackAllocatorFactory();

		if (!pStackAllocatorFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pStackAllocatorFactoryInstance->Init();

		if (result != RC_OK)
		{
			delete pStackAllocatorFactoryInstance;

			pStackAllocatorFactoryInstance = nullptr;
		}

		return dynamic_cast<IAllocatorFactory*>(pStackAllocatorFactoryInstance);
	}
}