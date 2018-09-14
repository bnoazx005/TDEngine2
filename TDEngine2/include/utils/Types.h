/*!
	\file Types.h
	\date 14.09.2018
	\authors Kasimov Ildar
*/


#pragma once


#include <cstdint>


namespace TDEngine2
{
	#if defined(_WIN32) || defined(_MSC_VER)
		#if defined(TDE2_DLLIMPORT)
			#define TDE2_API __declspec(dllimport)
		#else
			#define TDE2_API __declspec(dllexport)
		#endif
	#elif defined(__GNUC__)	
		#if defined(TDE2_DLLIMPORT)
		#define TDE2_API 
		#else
		#define TDE2_API __attribute__((visibility("default")))
		#endif
	#else /// Unknown platform and compiler
		#define TDE2_API 
	#endif

	
	/// Integral types

	typedef int8_t   I8;
	typedef int16_t  I16;
	typedef int32_t  I32;
	typedef int64_t  I64;
	typedef uint8_t  U8;
	typedef uint16_t U16;
	typedef uint32_t U32;
	typedef uint64_t U64;

	/// Floating-point types

	typedef float  F32;
	typedef double F64;

	/// Symbolic types

	typedef char     C8;
	typedef char16_t C16;
	typedef char32_t C32;

	/// Error codes

	/*!
		enum E_RESULT_CODE

		\brief The enumeration contains all the codes that can be returned as execution's result
	*/

	enum E_RESULT_CODE : uint32_t
	{
		EC_OK,							/// Normal execution
		EC_FAIL,						/// An execution failed at some point
		EC_INVALID_ARGS,				/// Some of input arguments are invalid or incorrect
		EC_UNKNOWN						/// Some unrecognized error
	};
}