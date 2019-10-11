#include "./../../include/utils/CU8String.h"


namespace TDEngine2
{
	CU8String::CU8String() :
		mpBuffer(nullptr), mCapacity(0), mBufferLength(0), mHasChanged(true)
	{
	}

	CU8String::CU8String(const C8* str) :
		mpBuffer(nullptr), mCapacity(strlen(str)), mBufferLength(strlen(str)), mHasChanged(true)
	{
		mpBuffer = new C8[mCapacity];

		memcpy(mpBuffer, str, mCapacity);
	}

	CU8String::CU8String(const std::string& str) :
		CU8String(str.c_str())
	{
	}

	CU8String::CU8String(const CU8String& str) :
		mpBuffer(nullptr), mCapacity(str.mCapacity), mBufferLength(str.mBufferLength), mHasChanged(true)
	{
		mpBuffer = new C8[mCapacity];

		memcpy(mpBuffer, str.mpBuffer, mCapacity);
	}

	CU8String::CU8String(CU8String&& str) :
		mpBuffer(str.mpBuffer), mCapacity(str.mCapacity), mBufferLength(str.mBufferLength), mHasChanged(true)
	{
	}

	CU8String::~CU8String()
	{
		if (mpBuffer)
		{
			delete[] mpBuffer;
		}
	}

	U32 CU8String::Length() const
	{
		return _getLength(mpBuffer, mBufferLength);
	}

	U8C CU8String::At(U32 pos) const
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

	U8 CU8String::GetHighSignificantByte(U8C codePoint)
	{
		U8 currByte = 0x0;
		U8 i = 1;

		while (!(currByte = static_cast<U8>(codePoint >> 8 * (4 - i))) && i <= 4)
		{
			i += 1;
		}

		return currByte;
	}

	U32 CU8String::GetCharLength(C8 ch)
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

	U32 CU8String::_getLength(const C8* pStr, U32 size) const
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

	U32 CU8String::_getInternalCharPos(const C8* pStrBuffer, U32 bufferSize, U32 pos) const
	{
		U32 internalPos = 0;

		for (int i = 0; i < pos && internalPos < bufferSize; ++i)
		{
			internalPos += GetCharLength(pStrBuffer[internalPos]);
		}

		return internalPos;
	}


	std::string UTF8CharToString(U8C ch)
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