/*!
\file CBaseAllocator.h
\date 31.10.2018
\authors Kasimov Ildar
*/

#pragma once


#include "CBaseAllocator.h"
#include "./../../utils/Utils.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CLinearAllocator's type.

		\param[in] totalMemorySize A value determines a size of a memory block

		\param[in, out] pMemoryBlock A pointer to a memory block

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CLinearAllocator's implementation
	*/

	TDE2_API IAllocator* CreateLinearAllocator(U32 totalMemorySize, U8* pMemoryBlock, E_RESULT_CODE& result);


	/*!
		class CLinearAllocator

		\brief The class implements a functionality of an allocator which
		allocates continuous picece of memory. This allocator cannot
		deallocate individually allocated memory. To free all used
		memory use Clear method
	*/

	class CLinearAllocator : public CBaseAllocator
	{
		public:
			friend TDE2_API IAllocator* CreateLinearAllocator(U32 totalMemorySize, U8* pMemoryBlock, E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(CLinearAllocator)

			/*!
				\brief The method initializes an internal state of an allocator

				\param[in] totalMemorySize A value determines a size of a memory block

				\param[in, out] pMemoryBlock A pointer to a memory block

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(U32 totalMemorySize, U8* pMemoryBlock) override;

			/*!
				\brief The method allocates a new piece of memory of specified size,
				which is aligned by aligment's value

				\param[in] size A size of an allocated memory

				\param[in] alignment An alignment of a block

				\return A pointer to the allocated block, returns nullptr if there is no free space
			*/

			TDE2_API void* Allocate(U32 size, U8 alignment) override;

			/*!
				\brief The method deallocates memory in position specified with a given pointer

				\param[in] pObjectPtr A pointer to piece of memory that should be freed

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Deallocate(void* pObjectPtr) override;

			/*!
				\brief The method clears up all used memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Clear() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CLinearAllocator)
		protected:
			void* mpCurrPos;
	};

	
	/*!
		\brief A factory function for creation objects of CLinearAllocatorFactory's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CLinearAllocatorFactory's implementation
	*/

	TDE2_API IAllocatorFactory* CreateLinearAllocatorFactory(E_RESULT_CODE& result);


	/*!
		class CLinearAllocatorFactory

		\brief The class is an implementation of IAllocatorFactory for CLinearAllocator
		type
	*/

	class CLinearAllocatorFactory : public CBaseAllocatorFactory
	{
		public:
			friend TDE2_API IAllocatorFactory* CreateLinearAllocatorFactory(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method creates a new instance of an allocator with a given parameters

				\param[in] pParams Parameters that are used for initialization of the allocator's instance

				\return Either a pointer to a new created IAllocator or an error code
			*/

			TDE2_API TResult<IAllocator*> Create(const TBaseAllocatorParams* pParams) const override;

			/*!
				\brief The method returns a type of an allocator that are created by the factory

				\return The method returns a type of an allocator that are created by the factory
			*/

			TDE2_API TypeId GetAllocatorType() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CLinearAllocatorFactory)
	};
}
