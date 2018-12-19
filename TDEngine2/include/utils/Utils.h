/*!
	\file Utils.h
	\date 02.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "Types.h"
#include <type_traits>
#include <atomic>


namespace TDEngine2
{
	/*!
		\brief The function is a common factory functor for all objects,
		which implements Init method (two-step initialization) instead of RAII.

		\param[in] args Arguments of an object that should be created (passed into its Init method)
		\param[out] result A result code of an execution process

		\return A pointer to I, which holds T implementation
	*/

	template <typename I, typename T, typename... TArgs>
#if _HAS_CXX17
	TDE2_API std::enable_if_t<std::is_base_of_v<I, T>, I*>
#else
	TDE2_API typename std::enable_if<std::is_base_of<I, T>::value, I*>::type
#endif 
	Create(TArgs&&... args, E_RESULT_CODE& result)
	{
		I* pInstance = new (std::nothrow) T();

		if (!pInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pInstance->Init(std::forward<TArgs>(args)...);

		return pInstance;
	}

	/*!
		\brief The function is a common factory functor for all objects,
		which implements Init method (two-step initialization) instead of RAII.

		\param[in] args Arguments of an object that should be created (passed into its Init method)
		\param[out] result A result code of an execution process

		\return A pointer to I, which holds T implementation
	*/

	template <typename I, typename T>
#if _HAS_CXX17
	TDE2_API std::enable_if_t<std::is_base_of_v<I, T>, I*> 
#else
	TDE2_API typename std::enable_if<std::is_base_of<I, T>::value, I*>::type 
#endif
	Create(E_RESULT_CODE& result)
	{
		I* pInstance = new (std::nothrow) T();

		if (!pInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pInstance->Init();

		return pInstance;
	}

	/*!
		\brief The function implements a generic mechanism of releasing memory.
		
		\param [in,out] pObject A reference to pointer to IBaseObjetc object, that should be released

		\return RC_OK if everything went ok, or some other code, which describes an error
	*/

	template <typename T>
	TDE2_API E_RESULT_CODE SafeFree(T*& pObject)
	{
		if (!pObject)
		{
			return RC_FAIL;
		}

		return pObject->Free();
	}


	/*!
		\brief The macros is used to declare protected members of interfaces, which
		should use two-step initialization technique
	*/

	#define DECLARE_INTERFACE_PROTECTED_MEMBERS(Type)		\
		TDE2_API Type() = default;							\
		TDE2_API virtual ~Type() = default;					\
		TDE2_API Type(const Type&) = delete;				\
		TDE2_API Type& operator= (Type&) = delete;

	/*!
		\brief The macros is used to declare protected members of interfaces' implementations, which
		should use two-step initialization technique
	*/

	#define DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(Type)		\
		TDE2_API Type();										\
		TDE2_API virtual ~Type() = default;						\
		TDE2_API Type(const Type&) = delete;					\
		TDE2_API Type& operator= (Type&) = delete;


	/*!
		\brief The method computes 32 bits hash based on an input string's value.
		The underlying algorithm's description can be found here
		http://www.cse.yorku.ca/~oz/hash.html

		\param[in] pStr An input string

		\param[in] hash The argument is used to store current hash value during a recursion

		\return 32 bits hash of the input string
	*/

	constexpr U32 ComputeHash(const C8* pStr, U32 hash = 5381)
	{
		return (*pStr != 0) ? ComputeHash(pStr + 1, ((hash << 5) + hash) + *pStr) : hash;
	}


	/*!
		\brief The macro is used to provide type counting mechanism

		All countable types should register themselves using it
	*/

#if defined (_MSC_VER)
	#define TDE2_REGISTER_TYPE(Type)								\
		static TypeId GetTypeId()									\
		{															\
			__pragma(warning(push))									\
			__pragma(warning(disable:4307))							\
			static constexpr TypeId typeId = ComputeHash(#Type);	\
			__pragma(warning(pop))									\
			return typeId;											\
		}

	#define TDE2_REGISTER_VIRTUAL_TYPE(Type)						\
		virtual TypeId GetTypeId() const							\
		{															\
			__pragma(warning(push))									\
			__pragma(warning(disable:4307))							\
			static constexpr TypeId typeId = ComputeHash(#Type);	\
			__pragma(warning(pop))									\
			return typeId;											\
		}
#else
	#define TDE2_REGISTER_TYPE(Type)								\
		static TypeId GetTypeId()									\
		{															\
			static constexpr TypeId typeId = ComputeHash(#Type);	\
			return typeId;											\
		}

#define TDE2_REGISTER_VIRTUAL_TYPE(Type)							\
		virtual TypeId GetTypeId() const							\
		{															\
			static constexpr TypeId typeId = ComputeHash(#Type);	\
			return typeId;											\
		}
#endif


	/*!
		\brief Endianness helper functions
	*/

	/*!
		\brief The function returns type of bytes order which is used on this host machine

		\return The function returns either ET_LITTLE_ENDIAN or ET_BIG_ENDIAN based on which one is used
		on the host
	*/

	TDE2_API E_ENDIAN_TYPE GetHostEndianType();

	/*!
		\brief The function swaps the order of bytes of a given value, type of which is represented as a word

		\param[in] value Any U16 value

		\return The U16 value with swapped order of bytes
	*/

	TDE2_API U16 Swap2Bytes(U16 value);

	/*!
		\brief The function swaps the order of bytes of a given value, type of which is a double word

		\param[in] value Any U32 value

		\return The U32 value with swapped order of bytes
	*/

	TDE2_API U32 Swap4Bytes(U32 value);

	/*!
		\brief The function swaps the order of bytes of a given value, type of which is a U64

		\param[in] value Any U64 value

		\return The U64 value with swapped order of bytes
	*/

	TDE2_API U64 Swap8Bytes(U64 value);
}