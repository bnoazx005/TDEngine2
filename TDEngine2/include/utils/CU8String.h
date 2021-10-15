/*!
	\file CU8String.h
	\date 05.07.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <string>
#include <cstring>
#include "Types.h"
#include "Config.h"


namespace TDEngine2
{
	typedef U32 U8C;


	/*!
		\brief The class is an implementation of UTF-8 encoded strings
		The class is header only

		\todo Add custom allocators instead of new usage
		\todo Move the definition of the class into CU8String.cpp file
	*/

	class CU8String
	{
		public:
			typedef USIZE TSizeType;
		public:
			TDE2_API CU8String();
			TDE2_API CU8String(const C8* str);
			TDE2_API CU8String(const std::string& str);
			TDE2_API CU8String(const CU8String& str);
			TDE2_API CU8String(CU8String&& str);
			TDE2_API ~CU8String();

			/*!
				\brief The method returns a length of a string in code points

				\return The method returns a length of a string in code points
			*/

			TDE2_API TSizeType Length() const;

			/*!
				\brief The method returns a code point at a given position

				\param[in] pos A position at which the code point should be returned

				\return The method returns a code point at a given position, returns an empty
				string if pos >= str.Length()
			*/

			TDE2_API U8C At(TSizeType pos) const;

			TDE2_API static U8 GetHighSignificantByte(U8C codePoint);

			/*!
				\brief The method returns a number of bytes which is occupied by the code point
				with specified high byte

				\param[in] ch A high byte of a code point

				\return A number of bytes which the code point occupies
			*/

			TDE2_API static U32 GetCharLength(C8 ch);
		protected:
			TDE2_API TSizeType _getLength(const C8* pStr, TSizeType size) const;
			TDE2_API TSizeType _getInternalCharPos(const C8* pStrBuffer, TSizeType bufferSize, TSizeType pos) const;
		protected:
			C8*  mpBuffer;

			TSizeType mCapacity;
			TSizeType mBufferLength;

			bool mHasChanged;
	};


	inline U8C StringToUTF8Char(const char* pStr)
	{
		return CU8String(pStr).At(0);
	}

	TDE2_API std::string UTF8CharToString(U8C ch);
}