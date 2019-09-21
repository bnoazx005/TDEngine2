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
			CU8String():
				mpBuffer(nullptr), mCapacity(0), mBufferLength(0), mHasChanged(true)
			{
			}

			CU8String(const C8* str):
				mpBuffer(nullptr), mCapacity(strlen(str)), mBufferLength(strlen(str)), mHasChanged(true)
			{
				mpBuffer = new C8[mCapacity];

				memcpy(mpBuffer, str, mCapacity);
			}

			CU8String(const std::string& str):
				CU8String(str.c_str())
			{
			}

			CU8String(const CU8String& str):
				mpBuffer(nullptr), mCapacity(str.mCapacity), mBufferLength(str.mBufferLength), mHasChanged(true)
			{
				mpBuffer = new C8[mCapacity];

				memcpy(mpBuffer, str.mpBuffer, mCapacity);
			}

			CU8String(CU8String&& str) :
				mpBuffer(str.mpBuffer), mCapacity(str.mCapacity), mBufferLength(str.mBufferLength), mHasChanged(true)
			{
			}

			~CU8String()
			{
				if (mpBuffer)
				{
					delete[] mpBuffer;
				}
			}

			/*!
				\brief The method returns a length of a string in code points

				\return The method returns a length of a string in code points
			*/

			U32 Length() const
			{
				return _getLength(mpBuffer, mBufferLength);
			}

			/*!
				\brief The method returns a code point at a given position

				\param[in] pos A position at which the code point should be returned

				\return The method returns a code point at a given position, returns an empty
				string if pos >= str.Length()
			*/

			U8C At(U32 pos) const
			{
				U32 internalPos = _getInternalCharPos(mpBuffer, mBufferLength, pos);

				if (internalPos >= mBufferLength)
				{
					return U8C();
				}

				U32 codePointLength = GetCharLength(mpBuffer[internalPos]);

				U8C result = 0x0;

				for (U32 i = 0; i < codePointLength; ++i)
				{
					result |= (static_cast<U8>(mpBuffer[internalPos + codePointLength - i - 1]) << (8 * i));
				}

				return result;
			}

			static U8 GetHighSignificantByte(U8C codePoint)
			{
				U8 currByte = 0x0;
				U8 i = 1;

				while (!(currByte = static_cast<U8>(codePoint >> 8 * (4 - i))) && i <= 4)
				{
					i += 1;
				}

				return currByte;
			}

			/*!
				\brief The method returns a number of bytes which is occupied by the code point
				with specified high byte

				\param[in] ch A high byte of a code point

				\return A number of bytes which the code point occupies
			*/

			static U32 GetCharLength(C8 ch)
			{
				if ((ch & 0xE0) == 0xC0)
				{
					return 2;
				}
				else if ((ch & 0xF0) == 0xE0)
				{
					return 3;
				}
				else if ((ch & 0xF8) == 0xF0)
				{
					return 4;
				}

				return 1;
			}
		protected:
			U32 _getLength(const C8* pStr, U32 size) const
			{
				U32 length = 0;

				U32 i = 0;

				while (i < size)
				{
					i += GetCharLength(pStr[i]);

					++length;
				}

				return length;
			}

			U32 _getInternalCharPos(const C8* pStrBuffer, U32 bufferSize, U32 pos) const
			{
				U32 internalPos = 0;

				for (int i = 0; i < pos && internalPos < bufferSize; ++i)
				{
					internalPos += GetCharLength(pStrBuffer[internalPos]);
				}

				return internalPos;
			}
		protected:
			C8*  mpBuffer;

			U32  mCapacity;

			U32  mBufferLength;

			bool mHasChanged;
	};


	inline U8C StringToUTF8Char(const char* pStr)
	{
		return CU8String(pStr).At(0);
	}

	inline std::string UTF8CharToString(U8C ch)
	{
		std::string str;

		if (ch < 0x7F)
		{
			str.push_back(static_cast<C8>(ch));
			return str;
		}

		U8 bytesCount = CU8String::GetCharLength(CU8String::GetHighSignificantByte(ch));
		
		for (U8 i = 0; i < bytesCount; ++i)
		{
			str.insert(str.begin(), ch & 0xFF);

			ch >>= 8;
		}

		return str;
	}
}