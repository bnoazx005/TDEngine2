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
	enum class TUtf8CodePoint : U32 { Invalid = (std::numeric_limits<U32>::max)() };


	/*!
		\brief The list of functions for work with UTF8 strings. By default it's assumed
		that base container for storage is std::string. These helpers should replace
		the previous implementation which was **CU8String**
	*/


	class CU8String
	{
		public:
			/*!
				\brief The method moves iterator to next code point and returns current one

				\brief[in, out] curr A current iterator that points to a code point
				\brief[in] endBound A right bound of a string which usually equals to str.end()
				\brief[out] codePoint Returns current code point
				
				\return Returns false if it was last element
			*/

			template <typename It>
			static bool MoveNext(It& curr, It endBound, TUtf8CodePoint& codePoint)
			{
				if (curr >= endBound)
				{
					return false;
				}

				const U32 codePointLength = GetCodePointLength(*curr);

				if (curr + static_cast<USIZE>(codePointLength) > endBound)
				{
					return false;
				}

				/// \note Extract current code point
				U32 result = 0x0;

				for (U32 i = 0; i < codePointLength; ++i)
				{
					result |= static_cast<U8>(*(curr + static_cast<USIZE>(codePointLength - i - 1))) << (8 * i);
				}

				/// \note If possible move iterator to next code point
				curr += static_cast<USIZE>(codePointLength);
				codePoint = TUtf8CodePoint(result);
				
				return true;
			}

			template <typename It>
			static USIZE Length(It begin, It end)
			{
				USIZE length = 0;

				for (It it = begin; it != end; it += GetCodePointLength(*it))
				{
					++length;
				}

				return length;
			}

			TDE2_API static U8 GetHighSignificantByte(TUtf8CodePoint codePoint);

			/*!
				\brief The method returns a number of bytes which is occupied by the code point
				based on its high byte

				\param[in] ch A high byte of a code point

				\return A number of bytes which the code point occupies
			*/

			TDE2_API static U32 GetCodePointLength(C8 ch);

			TDE2_API static std::string InsertAt(const std::string& str, USIZE codePointIndex, TUtf8CodePoint cp);

			TDE2_API static std::string EraseAt(const std::string& str, USIZE codePointIndex);

			TDE2_API static TUtf8CodePoint StringToUTF8CodePoint(const std::string& str);

			TDE2_API static std::string UTF8CodePointToString(TUtf8CodePoint cp);

			TDE2_API static std::string WCharArrayToString(const wchar_t* pStr);
	};
}