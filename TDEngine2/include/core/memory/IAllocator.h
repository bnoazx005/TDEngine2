/*!
	\file IAllocator.h
	\date 31.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../IBaseObject.h"
#include "../../utils/Utils.h"


namespace TDEngine2
{
	/*!
		interface IAllocator

		\brief The interface describes a functionality of a memory allocator
	*/

	class IAllocator: public virtual IBaseObject
	{
		public:
			typedef USIZE TSizeType;
		public:
			/*!
				\brief The method initializes an internal state of an allocator

				\param[in] totalMemorySize A value determines a size of a memory block

				\param[in, out] pMemoryBlock A pointer to a memory block

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(TSizeType totalMemorySize, U8* pMemoryBlock) = 0;

			/*!
				\brief The method allocates a new piece of memory of specified size,
				which is aligned by aligment's value

				\param[in] size A size of an allocated memory

				\param[in] alignment An alignment of a block

				\return A pointer to the allocated block, returns nullptr if there is no free space
			*/

			TDE2_API virtual void* Allocate(TSizeType size, U8 alignment) = 0;

			/*!
				\brief The method deallocates memory in position specified with a given pointer

				\param[in] pObjectPtr A pointer to piece of memory that should be freed

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Deallocate(void* pObjectPtr) = 0;

			/*!
				\brief The method clears up all used memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Clear() = 0;

#if TDE2_EDITORS_ENABLED
			TDE2_API virtual void SetBlockDebugName(const std::string& blockId) = 0;
#endif

			/*!
				\brief The method returns total size of available memory 

				\return The method returns total size of available memory 
			*/

			TDE2_API virtual TSizeType GetTotalMemorySize() const = 0;

			/*!
				\brief The method returns a size of used memory

				\return The method returns a size of used memory
			*/

			TDE2_API virtual TSizeType GetUsedMemorySize() const = 0;

			/*!
				\brief The methods returns total number of allocations

				\return The methods returns total number of allocations
			*/

			TDE2_API virtual U32 GetAllocationsCount() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IAllocator)
	};


	/*!
		struct TBaseAllocatorParams

		\brief The structure contains common parameters that are used to
		initialize a new allocator's instance
	*/

	typedef struct TBaseAllocatorParams
	{
		TDE2_API TBaseAllocatorParams() = default;
		TDE2_API TBaseAllocatorParams(USIZE size);
		TDE2_API virtual ~TBaseAllocatorParams() = default;

		USIZE mMemoryBlockSize;
	} TBaseAllocatorParams, *TBaseAllocatorParamsPtr;


	/*!
		interface IAllocatorFactory

		\brief The interface represents a factory of IAllocator objects.
		The interface is used to create new allocators using the custom memory manager
		which is defined in CMemoryManager.h file.
	*/

	class IAllocatorFactory: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes an internal state of an allocator factory
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;
			
			/*!
				\brief The method creates a new instance of an allocator with a given parameters

				\param[in, out] pMemoryBlock A pointer to a block of memory
				\param[in] params Parameters that are used for initialization of the allocator's instance

				\return Either a pointer to a new created IAllocator or an error code
			*/

			TDE2_API virtual TResult<IAllocator*> Create(U8* pMemoryBlock, const TBaseAllocatorParams& params) const = 0;

			/*!
				\brief The method returns a type of an allocator that are created by the factory

				\return The method returns a type of an allocator that are created by the factory
			*/

			TDE2_API virtual TypeId GetAllocatorType() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IAllocatorFactory)
	};
}
