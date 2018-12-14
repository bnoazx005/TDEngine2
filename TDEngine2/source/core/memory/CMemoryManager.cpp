#include "./../../../include/core/memory/CMemoryManager.h"
#include "./../../../include/utils/CFileLogger.h"
#include "./../../../include/core/memory/IAllocator.h"
#include "./../../../include/core/memory/CLinearAllocator.h"
#include "./../../../include/core/memory/CPoolAllocator.h"
#include "./../../../include/core/memory/CStackAllocator.h"
#include <cstring>


namespace TDEngine2
{
	CMemoryManager::CMemoryManager():
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CMemoryManager::Init(U32 totalMemorySize)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mpGlobalMemoryBlock = new U8[totalMemorySize];

		memset(mpGlobalMemoryBlock, 0, sizeof(U8) * totalMemorySize);

		LOG_MESSAGE("[Memory manager] The memory manager was successfully initialized...");
		LOG_MESSAGE("[Memory manager] Total pre-allocated memory block's size : "  + std::to_string(totalMemorySize / 1024) + " KiB");

		E_RESULT_CODE result = RC_OK;

		mpGlobalAllocator = CreateStackAllocator(totalMemorySize, mpGlobalMemoryBlock, result);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}
	
	E_RESULT_CODE CMemoryManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		_checkUpMemoryLeaks();

		E_RESULT_CODE result = RC_OK;

		if ((result = mpGlobalAllocator->Free()) != RC_OK)
		{
			return result;
		}

		delete[] mpGlobalMemoryBlock;

		LOG_MESSAGE("[Memory manager] The memory manager was successfully finalized");

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CMemoryManager::RegisterFactory(const IAllocatorFactory* pAllocatorFactory)
	{
		if (!pAllocatorFactory)
		{
			return RC_INVALID_ARGS;
		}

		TypeId allocatorType = pAllocatorFactory->GetAllocatorType();

		const IAllocatorFactory* pFactoryIter = mAllocatorFactories[allocatorType];

		E_RESULT_CODE result = RC_OK;

		if (pFactoryIter) /// if there is already attached factory, firstly we should unregister it
		{
			if ((result = _unregisterFactory(allocatorType)) != RC_OK)
			{
				return result;
			}
		}

		mAllocatorFactories[allocatorType] = pAllocatorFactory;

		return RC_OK;
	}

	void* CMemoryManager::Allocate(U32 size, const C8* userName)
	{
		void* pMemoryBlock = mpGlobalAllocator->Allocate(size, __alignof(U8));

		mAllocatedBlocks.emplace_back(userName, pMemoryBlock);

		std::string message = "[Memory manager] A new piece of memory was allocated ( allocation-size : ";

		message.append(std::to_string(size / 1024)).
				append(" KiB; user : ").
				append(!userName ? "unknown" : userName).
				append(")");

		LOG_MESSAGE(message);

		return pMemoryBlock;
	}

	E_RESULT_CODE CMemoryManager::Deallocate(void* pMemoryBlock)
	{
		if (!pMemoryBlock)
		{
			return RC_INVALID_ARGS;
		}

		const C8* userName = nullptr;

		void* pUsedMemoryBlock = nullptr;

		std::tie(userName, pUsedMemoryBlock) = mAllocatedBlocks.back();

		if (pUsedMemoryBlock != pMemoryBlock)
		{
			/// cannot free the memory because given memory block doesn't lie on top of the stack (global allocator is a stack allocator)

			return RC_FAIL;
		}

		mAllocatedBlocks.pop_back();

		return RC_OK;
	}
		
	void CMemoryManager::_checkUpMemoryLeaks()
	{
		if (mAllocatedBlocks.empty())
		{
			return;
		}

		LOG_WARNING("                      ***                ");
		LOG_WARNING("[Memory manager] Memory leaks detected!!!");
		LOG_WARNING("                      ***                ");
		
		const C8* userName = nullptr;

		void* pUsedMemoryBlock = nullptr;
		
		std::string message;

		for (auto iter = mAllocatedBlocks.rbegin(); iter != mAllocatedBlocks.rend(); ++iter)
		{
			std::tie(userName, pUsedMemoryBlock) = *iter;

			message = "[Memory manager] A memory leak was detected (user : ";
			
			message.append(!userName ? "unknown" : userName).
					append("; address : ").
					append(std::to_string((U32Ptr)pUsedMemoryBlock)).
					append(")");

			LOG_WARNING(message);
		}
	}

	E_RESULT_CODE CMemoryManager::_unregisterFactory(TypeId factoryTypeId)
	{
		TAllocatorFactoryHashTable::const_iterator factoryIter = mAllocatorFactories.find(factoryTypeId);

		if (factoryIter == mAllocatorFactories.cend())
		{
			return RC_FAIL;
		}

		mAllocatorFactories.erase(factoryIter);

		return RC_OK;
	}

	IAllocator* CMemoryManager::_createAllocator(TypeId allocatorTypeId, U32 size, const C8* userName)
	{
		const IAllocatorFactory* pAllocatorFactory = mAllocatorFactories[allocatorTypeId];

		if (!pAllocatorFactory)
		{
			return nullptr;
		}

		TBaseAllocatorParams params;
		
		params.mMemoryBlockSize = size;
		params.mpMemoryBlock    = static_cast<U8*>(Allocate(size, userName));

		TResult<IAllocator*> result = pAllocatorFactory->Create(&params);

		return result.Get();
	}

	E_ENGINE_SUBSYSTEM_TYPE CMemoryManager::GetType() const
	{
		return EST_MEMORY_MANAGER;
	}


	TDE2_API IMemoryManager* CreateMemoryManager(U32 totalMemorySize, E_RESULT_CODE& result)
	{
		CMemoryManager* pMemoryManager = new (std::nothrow) CMemoryManager();

		if (!pMemoryManager)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pMemoryManager->Init(totalMemorySize);

		if (result != RC_OK)
		{
			delete pMemoryManager;

			pMemoryManager = nullptr;
		}

		return dynamic_cast<IMemoryManager*>(pMemoryManager);
	}
}