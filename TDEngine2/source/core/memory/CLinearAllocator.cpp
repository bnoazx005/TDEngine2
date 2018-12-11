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

		U32Ptr alignedAddress = reinterpret_cast<U32Ptr>(mpCurrPos) + padding;

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


	CLinearAllocatorFactory::CLinearAllocatorFactory():
		CBaseAllocatorFactory()
	{
	}

	TResult<IAllocator*> CLinearAllocatorFactory::Create(const TBaseAllocatorParams* pParams) const
	{
		if (!pParams)
		{
			return TErrorValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		E_RESULT_CODE result = RC_OK;

		IAllocator* pAllocator = CreateLinearAllocator(pParams->mMemoryBlockSize, pParams->mpMemoryBlock, result);

		if (result != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		return TOkValue<IAllocator*>(pAllocator);
	}

	TypeId CLinearAllocatorFactory::GetAllocatorType() const
	{
		return CLinearAllocator::GetTypeId();
	}


	TDE2_API IAllocatorFactory* CreateLinearAllocatorFactory(E_RESULT_CODE& result)
	{
		CLinearAllocatorFactory* pLinearAllocatorFactoryInstance = new (std::nothrow) CLinearAllocatorFactory();

		if (!pLinearAllocatorFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pLinearAllocatorFactoryInstance->Init();

		if (result != RC_OK)
		{
			delete pLinearAllocatorFactoryInstance;

			pLinearAllocatorFactoryInstance = nullptr;
		}

		return dynamic_cast<IAllocatorFactory*>(pLinearAllocatorFactoryInstance);
	}
}