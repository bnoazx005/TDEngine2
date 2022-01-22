#include "./../../include/utils/CU8String.h"
#include "./../../include/core/IInputContext.h"


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

	CU8String::CU8String(const wchar_t* wstr) :
		mpBuffer(nullptr), mCapacity(wcslen(wstr) * 2), mBufferLength(wcslen(wstr) * 2), mHasChanged(true)
	{
		mpBuffer = new C8[mCapacity];
		wcstombs(mpBuffer, wstr, mCapacity);
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

	CU8String::TSizeType CU8String::Length() const
	{
		return _getLength(mpBuffer, mBufferLength);
	}

	U8C CU8String::At(TSizeType pos) const
	{
		TSizeType internalPos = _getInternalCharPos(mpBuffer, mBufferLength, pos);

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

	CU8String::TSizeType CU8String::_getLength(const C8* pStr, TSizeType size) const
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

	CU8String::TSizeType CU8String::_getInternalCharPos(const C8* pStrBuffer, TSizeType bufferSize, TSizeType pos) const
	{
		U32 internalPos = 0;

		for (U32 i = 0; i < pos && internalPos < bufferSize; ++i)
		{
			internalPos += GetCharLength(pStrBuffer[internalPos]);
		}

		return internalPos;
	}


	TDE2_API CU8String operator"" _U8Str(const C8* pStr, USIZE)
	{
		return CU8String(pStr);
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


	TDE2_API U8C KeyCodeToUTF8Char(const E_KEYCODES& keyCode)
	{
		constexpr U16 minAlphaValue = static_cast<U16>(E_KEYCODES::KC_ALPHA0);
		constexpr U16 maxAlphaValue = static_cast<U16>(E_KEYCODES::KC_ALPHA9);
		static_assert(minAlphaValue < maxAlphaValue, "Expect that KC_ALPHA0 is lesser than KC_ALPHA9. Please, don't change their order");

		constexpr U16 minCharValue = static_cast<U16>(E_KEYCODES::KC_A);
		constexpr U16 maxCharValue = static_cast<U16>(E_KEYCODES::KC_Z);
		static_assert(minCharValue < maxCharValue, "Expect that characters key codes are lexicographically ordered. Please, don't change their order");

		U16 convertedKeyCode = static_cast<U16>(keyCode);

		if (convertedKeyCode >= minAlphaValue && convertedKeyCode <= maxAlphaValue)
		{
			return '0' + convertedKeyCode - minAlphaValue;
		}

		if (convertedKeyCode >= minCharValue && convertedKeyCode <= maxCharValue)
		{
			return 'a' + convertedKeyCode - minCharValue;
		}

		switch (keyCode)
		{
			case E_KEYCODES::KC_SPACE:
				return ' ';
			case E_KEYCODES::KC_EXCLAIM:
				return '!'; 
			case E_KEYCODES::KC_AT:
				return '@';
			case E_KEYCODES::KC_DOUBLE_QUOTE:
				return '\"'; 
			case E_KEYCODES::KC_HASH:
				return '#'; 
			case E_KEYCODES::KC_DOLLAR:
				return '$'; 
			case E_KEYCODES::KC_PERCENT:
				return '%'; 
			case E_KEYCODES::KC_AMPERSAND:
				return '&';
			case E_KEYCODES::KC_QUOTE:
				return '\'';
			case E_KEYCODES::KC_LPAREN:
				return '('; 
			case E_KEYCODES::KC_RPAREN:
				return ')'; 
			case E_KEYCODES::KC_ASTERISK:
				return '*'; 
			case E_KEYCODES::KC_PLUS:
				return '+'; 
			case E_KEYCODES::KC_MINUS:
				return '-'; 
			case E_KEYCODES::KC_EQUALS:
				return '=';
			case E_KEYCODES::KC_COMMA:
				return ','; 
			case E_KEYCODES::KC_PERIOD:
				return '.'; 
			case E_KEYCODES::KC_SLASH:
				return '/'; 
			case E_KEYCODES::KC_BACKSLASH:
				return '\\'; 
			case E_KEYCODES::KC_COLON:
				return ':'; 
			case E_KEYCODES::KC_SEMICOLON:
				return ';'; 
			case E_KEYCODES::KC_LESS:
				return '<';
			case E_KEYCODES::KC_GREATER:
				return '>'; 
			case E_KEYCODES::KC_QUESTION:
				return '?'; 
			case E_KEYCODES::KC_LBRACKET:
				return '['; 
			case E_KEYCODES::KC_RBRACKET:
				return ']'; 
			case E_KEYCODES::KC_CARET:
				return '^'; 
			case E_KEYCODES::KC_UNDERSCORE:
				return '_'; 
			case E_KEYCODES::KC_BACK_QUOTE:
				return '\"';
			case E_KEYCODES::KC_LCURLY_BRACKET:
				return '{'; 
			case E_KEYCODES::KC_RCURLY_BRACKET:
				return '}'; 
			case E_KEYCODES::KC_PIPE:
				return '|'; 
			case E_KEYCODES::KC_TILDE:
				return '~';
		}

		return convertedKeyCode;
	}


	bool IsAlphaNumericKeyCode(const E_KEYCODES& keyCode)
	{
		return (keyCode >= E_KEYCODES::KC_A && keyCode <= E_KEYCODES::KC_Z) ||
			(keyCode >= E_KEYCODES::KC_ALPHA0 && keyCode <= E_KEYCODES::KC_NUMPAD9) ||
			(keyCode >= E_KEYCODES::KC_EXCLAIM && keyCode <= E_KEYCODES::KC_TILDE);
	}
}