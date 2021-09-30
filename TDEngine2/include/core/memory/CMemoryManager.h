/*!
	\file CMemoryManager.h
	\date 09.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IMemoryManager.h"
#include "../CBaseObject.h"
#include <vector>
#include <tuple>
#include <unordered_map>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CMemoryManager's type.

		\param[in] totalMemorySize A value determines a size of a memory block

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CMemoryManager's implementation
	*/

	TDE2_API IMemoryManager* CreateMemoryManager(U32 totalMemorySize, E_RESULT_CODE& result);


	/*!
		class CMemoryManager;

		\brief The class represents a global memory manager
	*/

	class CMemoryManager : public IMemoryManager, public CBaseObject
	{
		public:
			friend TDE2_API IMemoryManager* CreateMemoryManager(U32 totalMemorySize, E_RESULT_CODE& result);
		public:
			typedef std::vector<std::tuple<const C8*, void*>> TMemoryBlocksTable;

			typedef std::unordered_map<TypeId, const IAllocatorFactory*> TAllocatorFactoryHashTable;
		public:
			/*!
				\brief The method initializes an internal state of an allocator

				\param[in] totalMemorySize A value determines a size of a global memory block

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(U32 totalMemorySize) override;

			/*!
				\brief The method registers a given factory object within the manager

				\param[in] pAllocatorFactory A pointer to IAlocatorFactory implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RegisterFactory(const IAllocatorFactory* pAllocatorFactory) override;

			/*!
				\brief The method allocates a new piece of memory from the global storage

				\param[in] size A size of a memory block should be allocated

				\param[in] userName An optional parameter that specifies a name of a user of the memory block

				\return A pointer to a new allocated memory block
			*/

			TDE2_API void* Allocate(U32 size, const C8* userName = nullptr) override;

			/*!
				\brief The method deallocates a given piece of memory

				\param[in] pMemoryBlock A pointer to memory block that was allocated by the manager

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Deallocate(void* pMemoryBlock) override;
			
			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CMemoryManager)

			TDE2_API void _checkUpMemoryLeaks();

			TDE2_API E_RESULT_CODE _unregisterFactory(TypeId factoryTypeId) override;

			TDE2_API IAllocator* _createAllocator(TypeId allocatorTypeId, const TBaseAllocatorParams& params, const C8* userName) override;

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			bool                       mIsInitialized;

			U8*                        mpGlobalMemoryBlock;

			IAllocator*                mpGlobalAllocator;

			TMemoryBlocksTable         mAllocatedBlocks;

			TAllocatorFactoryHashTable mAllocatorFactories;
	};
}