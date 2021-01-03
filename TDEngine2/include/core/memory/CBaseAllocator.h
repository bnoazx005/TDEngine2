/*!
	\file CBaseAllocator.h
	\date 31.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../CBaseObject.h"
#include "IAllocator.h"


namespace TDEngine2
{
	/*!
		class CBaseAllocator

		\brief The class implements a common functionality of all memory allocators that are 
		used in the engine

		The allocator's implementation is based on following article:
		https://www.gamedev.net/articles/programming/general-and-gameplay-programming/c-custom-memory-allocation-r3010/
	*/

	class CBaseAllocator : public CBaseObject, public IAllocator
	{
		public:
			/*!
				\brief The method initializes an internal state of an allocator

				\param[in] totalMemorySize A value determines a size of a memory block

				\param[in, out] pMemoryBlock A pointer to a memory block

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(U32 totalMemorySize, U8* pMemoryBlock) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method allocates a new piece of memory of specified size,
				which is aligned by aligment's value

				\param[in] size A size of an allocated memory

				\param[in] alignment An alignment of a block

				\return A pointer to the allocated block, returns nullptr if there is no free space
			*/

			TDE2_API virtual void* Allocate(U32 size, U8 alignment) = 0;

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
			TDE2_API void SetBlockDebugName(const std::string& blockId) override;
#endif

			/*!
				\brief The method returns total size of available memory

				\return The method returns total size of available memory
			*/

			TDE2_API U32 GetTotalMemorySize() const override;

			/*!
				\brief The method returns a size of used memory

				\return The method returns a size of used memory
			*/

			TDE2_API U32 GetUsedMemorySize() const override;
			
			/*!
				\brief The methods returns total number of allocations

				\return The methods returns total number of allocations
			*/

			TDE2_API U32 GetAllocationsCount() const override;

			/*!
				\brief The method returns an aligned address. If current address
				is already aligned then it returns without any change. In other 
				case nearest forward address will be returned which is a multiple
				of alignment value

				\param[in] An input memory address
				
				\param[in] An alignment value (should be power of 2)

				\return An aligned value
			*/

			TDE2_API static void* GetAlignedAddress(void* pAddress, U8 alignment);

			/*!
				\brief The method returns a padding value that should be added to
				current given address to make it aligned

				\param[in] An input memory address

				\param[in] An alignment value (should be power of 2)

				\return The method returns a padding value that should be added to
				current given address to make it aligned
			*/

			TDE2_API static U8 GetPadding(void* pAddress, U8 alignment);

			/*!
				\brief The method returns a padding value that should be added to
				current given address to make it aligned

				\param[in] An input memory address

				\param[in] An alignment value (should be power of 2)

				\return The method returns a padding value that should be added to
				current given address to make it aligned
			*/

			TDE2_API static U8 GetPaddingWithHeader(void* pAddress, U8 alignment, U8 headerSize);
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseAllocator)
		protected:
			U32 mTotalMemorySize;

			U32 mUsedMemorySize;

			U32 mAllocationsCount;

			U8* mpMemoryBlock;

#if TDE2_EDITORS_ENABLED
			std::string mName;
#endif
	};


	/*!
		\brief The function allocates a new chunk of aligned memory of given size

		\param[in, out] pAllocator A pointer to an allocator's implementation

		\param[in] size A memory's size

		\param[in] alignment An alignment of a memory block

		\return The method returns a pointer to an allocated memory block. If pAlignment equals to nullptr
		or there is no free space within memory then nullptr will be returned
	*/

	TDE2_API void* AllocateMemory(IAllocator* pAllocator, U32 size, U32 alignment);


	/*!
		class CBaseAllocatorFactory

		\brief The abstract class is used to derive new types of allocators' factories
	*/

	class CBaseAllocatorFactory : public CBaseObject, public IAllocatorFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of an allocator factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseAllocatorFactory)
	};
}
