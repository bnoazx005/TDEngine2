/*!
	\file IMemoryManager.h
	\date 09.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../IEngineSubsystem.h"
#include "./../../utils/Types.h"
#include "./../../utils/Utils.h"


namespace TDEngine2
{
	class IAllocator;
	class IAllocatorFactory;
	

	/*!
		interface IMemoryManager;

		\brief The interface describes a functionality of a
		memory manager, which creates / destroys allocators
		of different types
	*/

	class IMemoryManager: public IEngineSubsystem
	{
		public:
			/*!
				\brief The method initializes an internal state of an allocator

				\param[in] totalMemorySize A value determines a size of a global memory block
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(U32 totalMemorySize) = 0;

			/*!
				\brief The method registers a given factory object within the manager

				\param[in] pAllocatorFactory A pointer to IAlocatorFactory implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE RegisterFactory(const IAllocatorFactory* pAllocatorFactory) = 0;
			
			/*!
				\brief The method unregisters a factory of a given type
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/
			template <typename T>
			TDE2_API
#if _HAS_CXX17
			std::enable_if_t<std::is_base_of_v<IAllocator, T>, E_RESULT_CODE>
#else
			typename std::enable_if<std::is_base_of<IAllocator, T>::value, E_RESULT_CODE>::type
#endif
			UnregisterFactory()
			{
				return _unregisterFactory(T::GetTypeId());
			}

			/*!
				\brief The method allocates a new piece of data of a specified size and creates
				a new allocator to manage it

				\param[in] size A size of a memory block should be allocated

				\param[in] userName An optional parameter that specifies a name of a user of the memory block

				\return A pointer to a new created IAllocator object
			*/

			template <typename T>
			TDE2_API
#if _HAS_CXX17
			std::enable_if_t<std::is_base_of_v<IAllocator, T>, IAllocator*>
#else
			typename std::enable_if<std::is_base_of<IAllocator, T>::value, IAllocator*>::type
#endif
			CreateAllocator(U32 size, const C8* userName)
			{
				return _createAllocator(T::GetTypeId(), size, userName);
			}

			/*!
				\brief The method allocates a new piece of memory from the global storage

				\param[in] size A size of a memory block should be allocated

				\param[in] userName An optional parameter that specifies a name of a user of the memory block

				\return A pointer to a new allocated memory block
			*/

			TDE2_API virtual void* Allocate(U32 size, const C8* userName = nullptr) = 0;

			/*!
				\brief The method deallocates a given piece of memory

				\param[in] pMemoryBlock A pointer to memory block that was allocated by the manager

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Deallocate(void* pMemoryBlock) = 0;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_MEMORY_MANAGER; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IMemoryManager)

			TDE2_API virtual E_RESULT_CODE _unregisterFactory(TypeId factoryTypeId) = 0;

			TDE2_API virtual IAllocator* _createAllocator(TypeId allocatorType, U32 size, const C8* userName) = 0;
	};
}