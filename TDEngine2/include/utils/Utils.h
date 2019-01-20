/*!
	\file Utils.h
	\date 02.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "Types.h"
#include <type_traits>
#include <atomic>
#include <string>


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


	/*!
		\brief Scoped enums helpers
	*/

	/*!
		struct TUsesBitmaskOperator

		\brief The structure is used to implement SFINAE concept
		which shows whether the type uses bitmasks operators or not
	*/

	template <typename T>
	struct TUsesBitmaskOperator
	{
		static const bool mValue = false;
	};

	/*!
		\brief The macro is used to enable the bitmask operator's overloading for
		the given enumeration's type. This macro is for INTERNAL usage only if 
		you need to use it in your project use TDE2_DECLARE_BITMASK_OPERATORS
	*/

#define TDE2_DECLARE_BITMASK_OPERATORS_INTERNAL(EnumType)	\
	template <>												\
	struct TUsesBitmaskOperator<EnumType>					\
	{														\
		static const bool mValue = true;					\
	};


	/*!
		\brief The macro is for external usage in your project that uses the engine
	*/

#define TDE2_DECLARE_BITMASK_OPERATORS(EnumType)	\
	namespace TDEngine2								\
	{												\
		template <>									\
		struct TUsesBitmaskOperator<EnumType>		\
		{											\
			static const bool mValue = true;		\
		};											\
	}


	/*!
		\brief The function is an overloading of bitmask | operator for enumerations that
		provide TUsesBitmaskOperator concept

		\param[in] left Left operand

		\param[in] right Right operand

		\return The function returns Enum's value which is a disjunction of left and right values
	*/

	template <typename Enum>
	TDE2_API 
#if _HAS_CXX17
	std::enable_if_t<TUsesBitmaskOperator<Enum>::mValue, Enum>
#else
	typename std::enable_if<TUsesBitmaskOperator<Enum>::mValue, Enum>::type
#endif 
	operator| (Enum left, Enum right)
	{
		using baseType = typename std::underlying_type<Enum>::type;

		return static_cast<Enum>(static_cast<baseType>(left) | static_cast<baseType>(right));
	}


	/*!
		\brief The function is an overloading of bitmask & operator for enumerations that
		provide TUsesBitmaskOperator concept

		\param[in] left Left operand

		\param[in] right Right operand

		\return The function returns Enum's value which is a conjunction of left and right values
	*/

	template <typename Enum>
	TDE2_API
#if _HAS_CXX17
		std::enable_if_t<TUsesBitmaskOperator<Enum>::mValue, Enum>
#else
		typename std::enable_if<TUsesBitmaskOperator<Enum>::mValue, Enum>::type
#endif 
	operator& (Enum left, Enum right)
	{
		using baseType = typename std::underlying_type<Enum>::type;

		return static_cast<Enum>(static_cast<baseType>(left) & static_cast<baseType>(right));
	}


	/*!
		\brief The function is an overloading of bitmask ^ operator for enumerations that
		provide TUsesBitmaskOperator concept

		\param[in] left Left operand

		\param[in] right Right operand

		\return The function returns Enum's value which is a XOR product of left and right values
	*/

	template <typename Enum>
	TDE2_API
#if _HAS_CXX17
		std::enable_if_t<TUsesBitmaskOperator<Enum>::mValue, Enum>
#else
		typename std::enable_if<TUsesBitmaskOperator<Enum>::mValue, Enum>::type
#endif 
		operator^ (Enum left, Enum right)
	{
		using baseType = typename std::underlying_type<Enum>::type;

		return static_cast<Enum>(static_cast<baseType>(left) ^ static_cast<baseType>(right));
	}


	/*!
		\brief String utils
		\todo Temporarily they are placed here, should be moved later
	*/


#if defined (_MSC_VER)
	#define TDE2_STRING_ID(Str)				\
		__pragma(warning(push))				\
		__pragma(warning(disable:4307))		\
		(TDEngine2::ComputeHash(Str))		\
		__pragma(warning(pop))
#else
	#define TDE2_STRING_ID(Str) (TDEngine2::ComputeHash(Str))
#endif


	/*!
		static class CStringUtils

		\brief The static class is a bunch of helper methods which are indended
		for strings processing
	*/

	class CStringUtils
	{
		public:
			/*!
				\brief The method processes a given string removing all single-line comments from it. The single-line comment's prefix
				can be defined via commentPrefixStr argument

				\param[in] source A processing string

				\param[in] commentPrefixStr A string that defines a prefix of single-line comment (for C-like languages it's '//',
				for Python '#' and etc) If the prefix is an empty string then source will be returned wihout any processing

				\return A processed string with no single-line comments
			*/

			TDE2_API static std::string RemoveSingleLineComments(const std::string& source, const std::string& commentPrefixStr = "//");

			/*!
				\brief The method processes a given string removing all multi-line comments from it. The multi-line comment's prefix
				and postfix can be defined via commentPrefixStr and commentPostfixStr arguments.

				\param[in] source A processing string

				\param[in] commentPrefixStr A string that defines a prefix of single-line comment.
				If the prefix is an empty string then source will be returned wihout any processing

				\param[in] commentPostfixStr A string that defines a postfix of single-line comment.
				If the postfix is an empty string then source will be returned wihout any processing
			*/

			TDE2_API static std::string RemoveMultiLineComments(const std::string& source, const std::string& commentPrefixStr = "/*",
																const std::string& commentPostfixStr = "*/");

			/*!
				\brief The method removes all extra whitespaces from a given string

				\param[in] str A processing string

				\return A processed string where a continuous sequence of whitespaces is replaced with a single one
			*/

			TDE2_API static std::string RemoveExtraWhitespaces(const std::string& str);

			/*!
				\brief The method returns an empty string

				\return The method returns an empty string
			*/

			TDE2_API static std::string GetEmptyStr();
		public:
			TDE2_API static const std::string mEmptyStr;
	};
}