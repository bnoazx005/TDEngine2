/*!
	\file CPoolAllocator.h
	\date 01.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseAllocator.h"
#include "../../utils/Utils.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CPoolAllocator's type.

		\param[in] objectSize A size of an object

		\param[in] objectAlignment An alignment of an object within a pool

		\param[in] pageSize The value determines an initial size of memory that's allowed to the allocator. Also it defines
		a size of newly allocated page when there is no enough space

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CPoolAllocator's implementation
	*/

	TDE2_API IAllocator* CreatePoolAllocator(USIZE objectSize, USIZE objectAlignment, USIZE pageSize, E_RESULT_CODE& result);


	/*!
		class CPoolAllocator

		\brief The class implements a functionality of an allocator which
		allocates piece of memory of same sizes
	*/

	class CPoolAllocator : public CBaseAllocator
	{
		public:
			friend TDE2_API IAllocator* CreatePoolAllocator(USIZE, USIZE, USIZE, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CPoolAllocator)

			/*!
				\brief The method initializes an internal state of an allocator

				\param[in] objectSize A size of an object

				\param[in] objectAlignment An alignment of an object within a pool

				\param[in] pageSize The value determines an initial size of memory that's allowed to the allocator. Also it defines
				a size of newly allocated page when there is no enough space

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(TSizeType objectSize, TSizeType objectAlignment, TSizeType pageSize);

			/*!
				\brief The method allocates a new piece of memory of specified size,
				which is aligned by aligment's value

				\param[in] size A size of an allocated memory

				\param[in] alignment An alignment of a block

				\return A pointer to the allocated block, returns nullptr if there is no free space
			*/

			TDE2_API void* Allocate(TSizeType size, U8 alignment) override;

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

			/*!
				\brief The method returns a size of used memory

				\return The method returns a size of used memory
			*/

			TDE2_API TSizeType GetUsedMemorySize() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPoolAllocator)

			TDE2_API void _clearMemoryRegion(TMemoryBlockEntity*& pRegion);
		protected:
			TSizeType mObjectSize;
			TSizeType mObjectAlignment;
			TSizeType mUsedMemorySize;

			void** mppNextFreeBlock;
	};


	/*!
		struct TPoolAllocatorParams

		\brief The structure contains parameters that are needed for
		CPoolAllocator's initialization
	*/

	typedef struct TPoolAllocatorParams: TBaseAllocatorParams
	{
		TDE2_API TPoolAllocatorParams() = default;
		TDE2_API TPoolAllocatorParams(USIZE size, USIZE perElementSize, USIZE elementAlignment);

		USIZE mPerObjectSize;
		USIZE mObjectAlignment;
	} TPoolAllocatorParams, *TPoolAllocatorParamsPtr;


	/*!
		\brief Derive your type from this below to provide custom pool allocator for it
	*/

	template <typename T, USIZE allocatorPageSize = 4096>
	class CPoolMemoryAllocPolicy
	{
		public:
			TDE2_API static void* operator new(std::size_t size, const std::nothrow_t&) { return _allocateImpl(size_t); }
			TDE2_API static void* operator new[](std::size_t size, const std::nothrow_t&) { return _allocateImpl(size_t); }

			TDE2_API static void operator delete(void* pPtr) noexcept { _deallocateImpl(pPtr); }
			TDE2_API static void operator delete[](void* pPtr) noexcept { _deallocateImpl(pPtr); }
		private:
			TDE2_API static void* _allocateImpl(std::size_t size)
			{
				if (auto pAllocator = _getAllocator())
				{
					return pAllocator->Allocate(size, alignof(T));
				}

				return nullptr;
			}

			TDE2_API static void _deallocateImpl(void* pPtr) noexcept
			{
				if (auto pAllocator = _getAllocator())
				{
					pAllocator->Deallocate(pPtr);
				}
			}

			TDE2_API TPtr<IAllocator> _getAllocator()
			{
				if (!mpTypeAllocator)
				{
					E_RESULT_CODE result = RC_OK;
					mpTypeAllocator = CreatePoolAllocator(sizeof(T), alignof(T), allocatorPageSize, result);
				}

				return mpTypeAllocator;
			}
		private:
			static TPtr<IAllocator> mpTypeAllocator;
	};

	template <typename T, USIZE allocatorPageSize> TPtr<IAllocator> CPoolMemoryAllocPolicy<T, allocatorPageSize>::mpTypeAllocator = TPtr<IAllocator>(nullptr);
}
