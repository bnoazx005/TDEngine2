/*!
	\file Utils.h
	\date 02.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "Types.h"
#include "result.hpp"
#include "memTracker.hpp"
#include <type_traits>
#include <atomic>
#include <string>
#include <cstring>
#include <vector>
#include <iostream>
#include <functional>
#include <algorithm>
#include <array>
#include <sstream>


namespace TDEngine2
{
	#define TDE2_NEW WRENCH_NEW ///< Our super custom memory trackable new implementation


#if defined(__unix__) || defined(unix)
	#if _HAS_CXX17
		#define TDE2_STATIC_CONSTEXPR static constexpr
	#else
		#define TDE2_STATIC_CONSTEXPR static const inline
	#endif
#else
	#define TDE2_STATIC_CONSTEXPR static constexpr
#endif


	/*!
		\brief The function is a common factory functor for all objects,
		which implements Init method (two-step initialization) instead of RAII.

		\param[in] typeFactory A functor that creates a new constructed object
		\param[in] typeFinalizer A functor-destructor
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
	CreateImpl(const std::function<T*()>& typeFactory, const std::function<void(T*&)>& typeFinalizer, E_RESULT_CODE& result, TArgs&&... args)
	{
		T* pInstance = typeFactory ? typeFactory() : nullptr;

		if (!pInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pInstance->Init(std::forward<TArgs>(args)...);

		if (result != RC_OK)
		{
			typeFinalizer(pInstance);
			pInstance = nullptr;
		}

		return dynamic_cast<I*>(pInstance);
	}

	/*!
		\brief The function is a common factory functor for all objects,
		which implements Init method (two-step initialization) instead of RAII.

		\param[in] typeFactory A functor that creates a new constructed object
		\param[in] typeFinalizer A functor-destructor
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
	CreateImpl(const std::function<T*()>& typeFactory, const std::function<void(T*&)>& typeFinalizer, E_RESULT_CODE& result)
	{
		T* pInstance = typeFactory ? typeFactory() : nullptr;

		if (!pInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pInstance->Init();

		if (result != RC_OK)
		{
			typeFinalizer(pInstance);
			pInstance = nullptr;
		}

		return dynamic_cast<I*>(pInstance);
	}


#if TDE2_EDITORS_ENABLED
	#define TDE2_REGISTER_BASE_OBJECT(Type, pPtr) do { if (pPtr) pPtr->RegisterObjectInProfiler(#Type); } while(false)
	#define TDE2_UNREGISTER_BASE_OBJECT(pPtr) do { if (pPtr) pPtr->OnBeforeMemoryRelease(); } while(false)
#else
	#define TDE2_REGISTER_BASE_OBJECT(Type, pPtr) do { } while(false)
	#define TDE2_UNREGISTER_BASE_OBJECT(pPtr) do { } while(false)
#endif


#define CREATE_IMPL(InterfaceType, ConcreteType, ...)																					\
	::TDEngine2::CreateImpl<InterfaceType, ConcreteType>(																				\
					[] { auto pPtr = new(std::nothrow) ConcreteType(); TDE2_REGISTER_BASE_OBJECT(ConcreteType, pPtr); return pPtr; },	\
					[](ConcreteType*& pPtr) { TDE2_UNREGISTER_BASE_OBJECT(pPtr); delete pPtr; }, __VA_ARGS__)


	template <typename T> 
	E_RESULT_CODE SafeFreeInternal(T*& pPtr, std::true_type)
	{
		if (!pPtr)
		{
			return RC_FAIL;
		}

		return pPtr->Free();
	}

	template <typename T>
	E_RESULT_CODE SafeFreeInternal(T*& pPtr, std::false_type)
	{
		return RC_OK;
	}


	/*!
		\brief The function implements a generic mechanism of releasing memory.
		
		\param [in,out] pObject A reference to pointer to IBaseObjetc object, that should be released

		\return RC_OK if everything went ok, or some other code, which describes an error
	*/

	template <typename T> E_RESULT_CODE SafeFree(T*& pObject) { return SafeFreeInternal<T>(pObject, std::is_base_of<class IBaseObject, T>{}); }


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
		\brief The macro is the same as DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS but with no destructor's default definition.
		It may be handy in case of std::unique_ptr usage with incomplete types
	*/

	#define DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS_NO_DCTR(Type)		\
		TDE2_API Type();												\
		TDE2_API Type(const Type&) = delete;							\
		TDE2_API Type& operator= (Type&) = delete;

	/*!
		\brief The macros is used to declare protected members of interfaces' implementations, which
		should use two-step initialization technique
	*/

	#define DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(Type)				\
		DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS_NO_DCTR(Type)			\
		TDE2_API virtual ~Type() = default;								


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


	template <typename T> struct GetTypeId { TDE2_API TDE2_STATIC_CONSTEXPR TypeId mValue = TypeId::Invalid; };


#define TDE2_REGISTER_CLASS_ID(ClassType)					\
	static const std::string& GetClassId()					\
	{														\
		static const std::string classId = #ClassType;		\
		return classId;										\
	}


	/*!
		\brief The macro is used to provide type counting mechanism

		All countable types should register themselves using it
	*/

#if defined (_MSC_VER)
#define TDE2_TYPE_ID(Type)												\
		__pragma(warning(push))											\
		__pragma(warning(disable:4307))									\
		::TDEngine2::TypeId(::TDEngine2::ComputeHash(#Type))			\
		__pragma(warning(pop))											


	#define TDE2_REGISTER_TYPE(Type)																			\
		static ::TDEngine2::TypeId GetTypeId()																	\
		{																										\
			__pragma(warning(push))																				\
			__pragma(warning(disable:4307))																		\
			static constexpr ::TDEngine2::TypeId typeId = ::TDEngine2::TypeId(::TDEngine2::ComputeHash(#Type));	\
			__pragma(warning(pop))																				\
			return typeId;																						\
		}																										\
		TDE2_REGISTER_CLASS_ID(Type)


#define TDE2_REGISTER_COMPONENT_TYPE(Type)									\
	TDE2_REGISTER_TYPE(Type)												\
	TDE2_REGISTER_VIRTUAL_TYPE_EX(Type, GetComponentTypeId)

	#define TDE2_REGISTER_VIRTUAL_TYPE_EX(Type, MethodName)														\
		virtual ::TDEngine2::TypeId MethodName() const															\
		{																										\
			__pragma(warning(push))																				\
			__pragma(warning(disable:4307))																		\
			static constexpr ::TDEngine2::TypeId typeId = ::TDEngine2::TypeId(::TDEngine2::ComputeHash(#Type));	\
			__pragma(warning(pop))																				\
			return typeId;																						\
		}
#else
	#define TDE2_TYPE_ID(Type) ::TDEngine2::TypeId(::TDEngine2::ComputeHash(#Type))	

	#define TDE2_REGISTER_TYPE(Type)																			\
		static ::TDEngine2::TypeId GetTypeId()																	\
		{																										\
			static constexpr ::TDEngine2::TypeId typeId = ::TDEngine2::TypeId(::TDEngine2::ComputeHash(#Type));	\
			return typeId;																						\
		}																										\
		TDE2_REGISTER_CLASS_ID(Type)


#define TDE2_REGISTER_COMPONENT_TYPE(Type)									\
		TDE2_REGISTER_TYPE(Type)											\
		TDE2_REGISTER_VIRTUAL_TYPE_EX(Type, GetComponentTypeId)

#define TDE2_REGISTER_VIRTUAL_TYPE_EX(Type, MethodName)															\
		virtual ::TDEngine2::TypeId MethodName() const															\
		{																										\
			static constexpr ::TDEngine2::TypeId typeId = ::TDEngine2::TypeId(::TDEngine2::ComputeHash(#Type));	\
			return typeId;																						\
		}
#endif


	template <> struct GetTypeId<F32> { TDE2_API TDE2_STATIC_CONSTEXPR TypeId mValue = TDE2_TYPE_ID(F32); };
	template <> struct GetTypeId<F64> { TDE2_API TDE2_STATIC_CONSTEXPR TypeId mValue = TDE2_TYPE_ID(F64); };
	template <> struct GetTypeId<I32> { TDE2_API TDE2_STATIC_CONSTEXPR TypeId mValue = TDE2_TYPE_ID(I32); };
	template <> struct GetTypeId<U32> { TDE2_API TDE2_STATIC_CONSTEXPR TypeId mValue = TDE2_TYPE_ID(U32); };
	template <> struct GetTypeId<bool> { TDE2_API TDE2_STATIC_CONSTEXPR TypeId mValue = TDE2_TYPE_ID(bool); };
	template <> struct GetTypeId<std::string> { TDE2_API TDE2_STATIC_CONSTEXPR TypeId mValue = TDE2_TYPE_ID(std::string); };



	/*!
		\brief Endianness helper functions
	*/

	/*!
		\brief The function returns type of bytes order which is used on this host machine

		\return The function returns either ET_LITTLE_ENDIAN or ET_BIG_ENDIAN based on which one is used
		on the host
	*/

	constexpr TDE2_API E_ENDIAN_TYPE GetHostEndianType()
	{
		U32 checker = 0x1;
		return (*((U8*)&checker) == 0x1) ? E_ENDIAN_TYPE::ET_LITTLE_ENDIAN : E_ENDIAN_TYPE::ET_BIG_ENDIAN;
	}

	
	template <typename T>
	T SwapBytesImpl(T value, std::integral_constant<USIZE, 1>)
	{
		return value;
	}


	template <typename T>
	T SwapBytesImpl(T value, std::integral_constant<USIZE, 2>)
	{
		typename std::make_unsigned<T>::type* pValue = reinterpret_cast<typename std::make_unsigned<T>::type*>(&value);
		return (*pValue & 0x00FF) << 8 | (*pValue & 0xFF00) >> 8;
	}


	template <typename T>
	T SwapBytesImpl(T value, std::integral_constant<USIZE, 4>)
	{
		typename std::make_unsigned<T>::type* pValue = reinterpret_cast<typename std::make_unsigned<T>::type*>(&value);
		return (*pValue & 0x000000FF) << 24 | (*pValue & 0x0000FF00) << 8 | (*pValue & 0x00FF0000) >> 8 | (*pValue & 0xFF000000) >> 24;
	}


	template <typename T>
	T SwapBytesImpl(T value, std::integral_constant<USIZE, 8>)
	{
		typename std::make_unsigned<T>::type* pValue = reinterpret_cast<typename std::make_unsigned<T>::type*>(&value);

		return (*pValue & 0x00000000000000FF) << 56 |
			(*pValue & 0x000000000000FF00) << 40 |
			(*pValue & 0x0000000000FF0000) << 24 |
			(*pValue & 0x00000000FF000000) << 8 |
			(*pValue & 0x000000FF00000000) >> 8 |
			(*pValue & 0x0000FF0000000000) >> 24 |
			(*pValue & 0x00FF000000000000) >> 40 |
			(*pValue & 0xFF00000000000000) >> 56;
	}


	template<typename T>
	T SwapBytes(T value) {
		static_assert(std::is_integral<T>::value, "SwapBytes<T> requires T to be an integral type.");
		return (E_ENDIAN_TYPE::ET_BIG_ENDIAN == GetHostEndianType()) ? SwapBytesImpl<T>(value, std::integral_constant<size_t, sizeof(T)>()) : value;
	}


	template <> TDE2_API F32 SwapBytes<F32>(F32 value);
	template <> TDE2_API F64 SwapBytes<F64>(F64 value);


	/*!
		\brief The functions swaps the order of bytes for a given object
	*/

	template <typename T>
	inline T SwapObjectBytes(T object)
	{
		T tmp(object);

		std::reverse(reinterpret_cast<U8*>(&tmp), reinterpret_cast<U8*>(&tmp) + sizeof(T));

		return tmp;
	}


	TDE2_API U8* SwapObjectBytes(U8* pPtr, U32 size);


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

	TDE2_DECLARE_BITMASK_OPERATORS_INTERNAL(E_RESULT_CODE);


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
	};


	TDE2_API void AssertImpl(const C8* message, const C8* file, I32 line);


	/*!
		\brief The following macro is used to mark some features as unimplemented. The invocations
		of the macro will cause interruption of execution's process
	*/

#define TDE2_UNIMPLEMENTED()													\
	do {																		\
		AssertImpl("The feature is not implemented yet", __FILE__, __LINE__);	\
	}																			\
	while (0)


	/*!
		\brief The macro is used to raise assertions when it's reached
	*/

#define TDE2_UNREACHABLE()													\
	do {																	\
		AssertImpl("Unreachable code was executed", __FILE__, __LINE__);	\
	}																		\
	while (0)


	class CFormatUtils
	{
		public:
			/*!
				\brief The function returns a size of a specified format

				\param[in] format An internal format

				\return A size in bytes of a specified format
			*/

			TDE2_API static U32 GetFormatSize(E_FORMAT_TYPE format);

			/*!
				\brief The function returns a number of channels for a given format type

				\param[in] format A specific format of a texture

				\return The functiona returns a number of channels for a given format type
			*/

			TDE2_API static U8 GetNumOfChannelsOfFormat(E_FORMAT_TYPE format);

			/*!
				\brief The function returns a format of E_FORMAT_TYPE which is parsed from
				incoming string representation

				\param[in] str A string that contains format's description

				\return An internal representation of data format
			*/

			TDE2_API static E_FORMAT_TYPE GetFormatFromString(const std::string& str);
	};


	/*!
		\brief The helper function is used to simplify code when it's needed to
		store either a valid pointer or some default alternative one

		\param[in] ptr A value should be stored
		\param[in] defaultPtr A value that will be returned if value isn't valid

		\return The function returns either a valid pointer or default one
	*/

	template <typename T>
	TDE2_API
#if _HAS_CXX17
	std::enable_if_t<std::is_pointer<T>::value, T>
#else
	typename std::enable_if<std::is_pointer<T>::value, T>::type
#endif  
	GetValidPtrOrDefault(T ptr, T defaultPtr)
	{
		return ptr ? ptr : defaultPtr;
	}



	/*!
		\brief The helper function is a custom,fast implementation of dynamic_cast
		operator which is based upon type_traits and static_cast. The main advantage over
		dynamic_cast is compile time validation of casting operation

		\param[in] pBase An input pointer to Base type

		\return The function returns casted pointer to Derived type which is a child class
		of Base type
	*/

	template <typename Derived, typename Base>
	Derived PolymorphicCast(Base* pBase)
	{
		static_assert(std::is_base_of<Base,	typename std::decay<Derived>::type>::value, "Invalid polymorphic cast");
		return static_cast<Derived>(pBase);
	}


#if defined (TDE2_DEBUG_MODE) || TDE2_PRODUCTION_MODE
	#define TDE2_ASSERT(expression) do { if (!(expression)) { TDEngine2::AssertImpl(#expression, __FILE__, __LINE__); } } while(0)
	#define TDE2_ASSERT_MSG(expression, message) do { if (!(expression)) { TDEngine2::AssertImpl(message, __FILE__, __LINE__); } } while(0)
#else
	#define TDE2_ASSERT(expression) 
	#define TDE2_ASSERT_MSG(expression, message)
#endif


	template <typename T> void ScopedPtrAdd(T*& pPtr) { pPtr->AddRef(); }
	template <typename T> void ScopedPtrRelease(T*& pPtr) { pPtr->Free(); }


#define TDE2_DECLARE_SCOPED_PTR(Type)						 \
	template <> TDE2_API void ScopedPtrAdd<Type>(Type*& pPtr);	 \
	template <> TDE2_API void ScopedPtrRelease<Type>(Type*& pPtr);


#define TDE2_DECLARE_SCOPED_PTR_INLINED(Type)										 \
	template <> TDE2_API inline void ScopedPtrAdd<Type>(Type*& pPtr) { pPtr->AddRef(); }	 \
	template <> TDE2_API inline void ScopedPtrRelease<Type>(Type*& pPtr) { pPtr->Free(); }


#define TDE2_DEFINE_SCOPED_PTR(Type)												 \
	template <> void ScopedPtrAdd<Type>(Type*& pPtr) { pPtr->AddRef(); }	 \
	template <> void ScopedPtrRelease<Type>(Type*& pPtr) { pPtr->Free(); }


	/*!
		class CScopedPtr<T>

		\brief The smart pointer that can be used with any type that's defined in TDEngine2 namespace
		and implements IBaseObject interface
	*/

	template <typename T>
	class CScopedPtr
	{
		public:
			CScopedPtr() : mpPtr(nullptr) {}
			CScopedPtr(const CScopedPtr<T>& ptr) : mpPtr(ptr.mpPtr) 
			{
				if (mpPtr)
				{
					ScopedPtrAdd<T>(mpPtr);
				}
			}

			CScopedPtr(CScopedPtr<T>&& ptr) : mpPtr(ptr.mpPtr) { ptr.mpPtr = nullptr; }
			explicit CScopedPtr(T* pPtr) : mpPtr(pPtr) {}
			CScopedPtr(std::nullptr_t) : mpPtr(nullptr) {}

			~CScopedPtr()
			{
				if (mpPtr)
				{
					ScopedPtrRelease<T>(mpPtr);
				}
			}

			T* operator->() const
			{
				TDE2_ASSERT(mpPtr);
				return mpPtr;
			}

			CScopedPtr<T>& operator= (CScopedPtr<T> ptr)
			{
				_swap(*this, ptr);
				return *this;
			}

			CScopedPtr<T>& operator= (T* pPtr)
			{
				CScopedPtr<T> temp(pPtr);
				_swap(*this, temp);

				return *this;
			}

			T* Get() { return mpPtr; }
			const T* Get() const { return mpPtr; }

			operator bool() const { return (mpPtr != nullptr); }

			bool operator== (const CScopedPtr<T>& other) const { return mpPtr == other.mpPtr; }
			bool operator!= (const CScopedPtr<T>& other) const { return mpPtr != other.mpPtr; }
		private:
			void _swap(CScopedPtr<T>& left, CScopedPtr<T>& right)
			{
				T* pPtr = left.mpPtr;
				left.mpPtr = right.mpPtr;
				right.mpPtr = pPtr;
			}

		private:
			T* mpPtr;
	};


	template <typename T> using TPtr = CScopedPtr<T>;


	template <typename T, typename U> 
	CScopedPtr<T> DynamicPtrCast(CScopedPtr<U> ptr) 
	{ 
		U* pPtr = ptr.Get(); 
		if (!pPtr) 
		{ 
			return CScopedPtr<T>(nullptr); 
		}

		if (pPtr)
		{
			pPtr->AddRef();
		}

		TDE2_ASSERT(dynamic_cast<T*>(pPtr));

		return CScopedPtr<T>(dynamic_cast<T*>(pPtr));
	}


	template <typename T, typename U>
	CScopedPtr<T> MakeScopedFromRawPtr(U* pPtr)
	{
		if (!pPtr)
		{
			return nullptr;
		}

		pPtr->AddRef();
		return CScopedPtr<T>(pPtr);
	}


	template <typename T> CScopedPtr<T> GetValidPtrOrDefault(CScopedPtr<T> ptr, CScopedPtr<T> defaultPtr) { return ptr ? ptr : defaultPtr; }


	/*!
		\brief The method returns 32 bits hash value which is unique for the current set of parameters
		\return The method returns 32 bits hash value which is unique for the current set of parameters
	*/

	template <typename T>
	U32 ComputeStateDescHash(const T& object)
	{
		TDE2_UNIMPLEMENTED();
		return 0;
	}


	template <> TDE2_API U32 ComputeStateDescHash<TBlendStateDesc>(const TBlendStateDesc& object);
	template <> TDE2_API U32 ComputeStateDescHash<TDepthStencilStateDesc>(const TDepthStencilStateDesc& object);
	template <> TDE2_API U32 ComputeStateDescHash<TTextureSamplerDesc>(const TTextureSamplerDesc& object);
	template <> TDE2_API U32 ComputeStateDescHash<TRasterizerStateDesc>(const TRasterizerStateDesc& object);


	/*!
		\brief The macro is used to define a base class for derived one
	*/

#define TDE2_BASE_CLASS(TClass) typedef TClass TBase


	/*!
		\brief The macro is used to terminate application if some fail occurs
	*/

#define PANIC_ON_FAILURE(expr) if ((expr) != RC_OK) { Wrench::Panic(#expr); }


	template <typename T> using TResult = Wrench::Result<T, E_RESULT_CODE>;


	template <typename T>
	inline TDE2_API
#if _HAS_CXX17
	std::enable_if_t<std::is_function<T>::value, void>
#else
	typename std::enable_if<std::is_function<T>::value, void>::type
#endif  
	SafeCallback(T callback)
	{
		if (!callback)
		{
			return;
		}

		callback();
	}


	template <typename... TArgs> 
	struct TContainsType 
	{
		static constexpr bool mValue = false;
	};

	template <typename TGivenType, typename TCurrentType, typename... TArgs>
	struct TContainsType<TGivenType, TCurrentType, TArgs...> 
	{
		static constexpr bool mValue = std::is_same<TGivenType, TCurrentType>::value || TContainsType<TGivenType, TArgs...>::mValue;
	};

	#define TDE2_STRINGIFY_IMPL(value) #value
	#define TDE2_STRINGIFY(...) TDE2_STRINGIFY_IMPL(__VA_ARGS__)

	#define TDE2_CONCAT_IMPL(A, B) A ## B
	#define TDE2_CONCAT(A, B) TDE2_CONCAT_IMPL(A, B)


#if TDE2_DEBUG_MODE

	class IGraphicsContext;
	TDE2_DECLARE_SCOPED_PTR(IGraphicsContext);


	/*!
		\brief The helper RAII type to declare named regions for GPU frame debuggers
	*/

	struct TGraphicsContextDebugRegion
	{
		TGraphicsContextDebugRegion(TPtr<IGraphicsContext> pGraphicsContext, const std::string& id);
		~TGraphicsContextDebugRegion();

		TPtr<IGraphicsContext> mpGraphicsContext;
	};

#define TDE_RENDER_SECTION(pGraphicsContext, id) TGraphicsContextDebugRegion TDE2_CONCAT(graphicsSection, __LINE__)(pGraphicsContext, id);
#else
#define TDE_RENDER_SECTION(pGraphicsContext, id) 
#endif
}