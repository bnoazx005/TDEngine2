#include "./../../include/utils/CU8String.h"
#include "./../../include/core/IInputContext.h"


namespace TDEngine2
{
	U8 CU8String::GetHighSignificantByte(TUtf8CodePoint codePoint)
	{
		U8 currByte = 0x0;
		U8 i = 1;

		while (!(currByte = static_cast<U8>(static_cast<U32>(codePoint) >> 8 * (4 - i))) && i <= 4)
		{
			i += 1;
		}

		return currByte;
	}

	U32 CU8String::GetCodePointLength(C8 ch)
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

	std::string CU8String::UTF8CodePointToString(TUtf8CodePoint cp)
	{
		std::string str;

		if (static_cast<C8>(cp) < 0x7F)
		{
			str.push_back(static_cast<C8>(cp));
			return str;
		}

		U32 codePointInternal = static_cast<U32>(cp);

		for (U8 i = 0; i < GetCodePointLength(GetHighSignificantByte(cp)); ++i)
		{
			str.insert(str.begin(), static_cast<U32>(codePointInternal) & 0xFF);

			codePointInternal >>= 8;
		}

		return str;
	}

	std::string CU8String::WCharArrayToString(const wchar_t* pStr)
	{
		const USIZE maxBytesCount = wcslen(pStr) * sizeof(char32_t);

		std::string outputString;
		outputString.reserve(maxBytesCount);

		wcstombs(&outputString.front(), pStr, maxBytesCount);

		return outputString;
	}


	TDE2_API TUtf8CodePoint KeyCodeToUTF8Char(const E_KEYCODES& keyCode)
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
			return static_cast<TUtf8CodePoint>('0' + convertedKeyCode - minAlphaValue);
		}

		if (convertedKeyCode >= minCharValue && convertedKeyCode <= maxCharValue)
		{
			return static_cast<TUtf8CodePoint>('a' + convertedKeyCode - minCharValue);
		}

		switch (keyCode)
		{
			case E_KEYCODES::KC_SPACE:
				return static_cast<TUtf8CodePoint>(' ');
			case E_KEYCODES::KC_EXCLAIM:
				return static_cast<TUtf8CodePoint>('!'); 
			case E_KEYCODES::KC_AT:
				return static_cast<TUtf8CodePoint>('@');
			case E_KEYCODES::KC_DOUBLE_QUOTE:
				return static_cast<TUtf8CodePoint>('\"'); 
			case E_KEYCODES::KC_HASH:
				return static_cast<TUtf8CodePoint>('#'); 
			case E_KEYCODES::KC_DOLLAR:
				return static_cast<TUtf8CodePoint>('$'); 
			case E_KEYCODES::KC_PERCENT:
				return static_cast<TUtf8CodePoint>('%'); 
			case E_KEYCODES::KC_AMPERSAND:
				return static_cast<TUtf8CodePoint>('&');
			case E_KEYCODES::KC_QUOTE:
				return static_cast<TUtf8CodePoint>('\'');
			case E_KEYCODES::KC_LPAREN:
				return static_cast<TUtf8CodePoint>('('); 
			case E_KEYCODES::KC_RPAREN:
				return static_cast<TUtf8CodePoint>(')'); 
			case E_KEYCODES::KC_ASTERISK:
				return static_cast<TUtf8CodePoint>('*'); 
			case E_KEYCODES::KC_PLUS:
				return static_cast<TUtf8CodePoint>('+'); 
			case E_KEYCODES::KC_MINUS:
				return static_cast<TUtf8CodePoint>('-'); 
			case E_KEYCODES::KC_EQUALS:
				return static_cast<TUtf8CodePoint>('=');
			case E_KEYCODES::KC_COMMA:
				return static_cast<TUtf8CodePoint>(','); 
			case E_KEYCODES::KC_PERIOD:
				return static_cast<TUtf8CodePoint>('.'); 
			case E_KEYCODES::KC_SLASH:
				return static_cast<TUtf8CodePoint>('/'); 
			case E_KEYCODES::KC_BACKSLASH:
				return static_cast<TUtf8CodePoint>('\\'); 
			case E_KEYCODES::KC_COLON:
				return static_cast<TUtf8CodePoint>(':'); 
			case E_KEYCODES::KC_SEMICOLON:
				return static_cast<TUtf8CodePoint>(');'); 
			case E_KEYCODES::KC_LESS:
				return static_cast<TUtf8CodePoint>('<');
			case E_KEYCODES::KC_GREATER:
				return static_cast<TUtf8CodePoint>('>'); 
			case E_KEYCODES::KC_QUESTION:
				return static_cast<TUtf8CodePoint>('?'); 
			case E_KEYCODES::KC_LBRACKET:
				return static_cast<TUtf8CodePoint>('['); 
			case E_KEYCODES::KC_RBRACKET:
				return static_cast<TUtf8CodePoint>(']'); 
			case E_KEYCODES::KC_CARET:
				return static_cast<TUtf8CodePoint>('^'); 
			case E_KEYCODES::KC_UNDERSCORE:
				return static_cast<TUtf8CodePoint>('_'); 
			case E_KEYCODES::KC_BACK_QUOTE:
				return static_cast<TUtf8CodePoint>('\"');
			case E_KEYCODES::KC_LCURLY_BRACKET:
				return static_cast<TUtf8CodePoint>('{'); 
			case E_KEYCODES::KC_RCURLY_BRACKET:
				return static_cast<TUtf8CodePoint>('}'); 
			case E_KEYCODES::KC_PIPE:
				return static_cast<TUtf8CodePoint>('|'); 
			case E_KEYCODES::KC_TILDE:
				return static_cast<TUtf8CodePoint>('~');
		}

		return static_cast<TUtf8CodePoint>(convertedKeyCode);
	}


	bool IsAlphaNumericKeyCode(const E_KEYCODES& keyCode)
	{
		return (keyCode >= E_KEYCODES::KC_A && keyCode <= E_KEYCODES::KC_Z) ||
			(keyCode >= E_KEYCODES::KC_ALPHA0 && keyCode <= E_KEYCODES::KC_NUMPAD9) ||
			(keyCode >= E_KEYCODES::KC_EXCLAIM && keyCode <= E_KEYCODES::KC_TILDE);
	}
}