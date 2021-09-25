#include "./../include/CUnixInputContext.h"
#include <core/IWindowSystem.h>
#include <platform/unix/CUnixWindowSystem.h>
#include <cstring>


#if defined (TDE2_USE_UNIXPLATFORM)

namespace TDEngine2
{
	CUnixInputContext::CUnixInputContext():
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CUnixInputContext::Init(IWindowSystem* pWindowSystem)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mpWindowSystem = pWindowSystem;

		CUnixWindowSystem* pUnixWindowSystem = dynamic_cast<CUnixWindowSystem*>(pWindowSystem);

		if (!pUnixWindowSystem)
		{
			return RC_INVALID_ARGS;	
		}

		mpDisplayHandler = pUnixWindowSystem->GetInternalData().mpDisplayHandler;

		mWindowHandler = pUnixWindowSystem->GetInternalData().mWindowHandler;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CUnixInputContext::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CUnixInputContext::Update()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mPrevCursorPos = mCurrCursorPos;

		mPrevMouseState = mCurrMouseState;

		I32 x, y, x1,y1;

		Window root, child;

		XQueryPointer(mpDisplayHandler, mWindowHandler, &root, &child, &x1, &y1, &x, &y, &mCurrMouseState);

		mCurrCursorPos.x = x;
		mCurrCursorPos.y = mpWindowSystem->GetHeight() - y;

		memcpy(mPrevKeysStates, mCurrKeysStates, sizeof(C8) * mMaxKeysBufferSize);

		XQueryKeymap(mpDisplayHandler, mCurrKeysStates);

		return RC_OK;
	}

	bool CUnixInputContext::IsKeyPressed(E_KEYCODES keyCode)
	{
		U8 internalKeycode = _toKeycode(keyCode);
		
		return (!_isKeyPressed(mPrevKeysStates, internalKeycode)) && (_isKeyPressed(mCurrKeysStates, internalKeycode));
	}

	bool CUnixInputContext::IsKey(E_KEYCODES keyCode)
	{
		return _isKeyPressed(mCurrKeysStates, _toKeycode(keyCode));		
	}

	bool CUnixInputContext::IsKeyUnpressed(E_KEYCODES keyCode)
	{
		U8 internalKeycode = _toKeycode(keyCode);
		
		return (_isKeyPressed(mPrevKeysStates, internalKeycode)) && (!_isKeyPressed(mCurrKeysStates, internalKeycode));
	}

	bool CUnixInputContext::IsMouseButtonPressed(U8 button)
	{
		return (!_isButtonPressed(mPrevMouseState, button)) && (_isButtonPressed(mCurrMouseState, button));
	}

	bool CUnixInputContext::IsMouseButton(U8 button)
	{
		return _isButtonPressed(mCurrMouseState, button);
	}

	bool CUnixInputContext::IsMouseButtonUnpressed(U8 button)
	{
		return (_isButtonPressed(mPrevMouseState, button)) && (!_isButtonPressed(mCurrMouseState, button));
	}

	TVector3 CUnixInputContext::GetMousePosition() const
	{
		return mCurrCursorPos;
	}

	TVector2 CUnixInputContext::GetNormalizedMousePosition() const
	{
		TDE2_UNIMPLEMENTED();
		TVector3 pos = GetMousePosition();
		return TVector2(pos.x, pos.y);
	}

	TVector3 CUnixInputContext::GetMouseShiftVec() const
	{
		return mCurrCursorPos - mPrevCursorPos;
	}

	E_ENGINE_SUBSYSTEM_TYPE CUnixInputContext::GetType() const
	{
		return EST_INPUT_CONTEXT;
	}

	const TInternalInputData& CUnixInputContext::GetInternalHandler() const
	{
		return mInternalData;
	}

	IGamepad* CUnixInputContext::GetGamepad(U8 gamepadId) const
	{
        return nullptr;
	}
	
	U8 CUnixInputContext::_toKeycode(E_KEYCODES keyCode)
	{
		switch (keyCode)
		{
			case E_KEYCODES::KC_ESCAPE:
				return 9;
			case E_KEYCODES::KC_ALPHA0:
				return 19;
			case E_KEYCODES::KC_ALPHA1:
				return 10;
			case E_KEYCODES::KC_ALPHA2:
				return 11;
			case E_KEYCODES::KC_ALPHA3:
				return 12;
			case E_KEYCODES::KC_ALPHA4:
				return 13;
			case E_KEYCODES::KC_ALPHA5:
				return 14;
			case E_KEYCODES::KC_ALPHA6:
				return 15;
			case E_KEYCODES::KC_ALPHA7:
				return 16;
			case E_KEYCODES::KC_ALPHA8:
				return 17;
			case E_KEYCODES::KC_ALPHA9:
				return 18;
			case E_KEYCODES::KC_NUMPAD0:
				return 90;
			case E_KEYCODES::KC_NUMPAD1:
				return 87;
			case E_KEYCODES::KC_NUMPAD2:
				return 88;
			case E_KEYCODES::KC_NUMPAD3:
				return 89;
			case E_KEYCODES::KC_NUMPAD4:
				return 83;
			case E_KEYCODES::KC_NUMPAD5:
				return 84;
			case E_KEYCODES::KC_NUMPAD6:
				return 85;
			case E_KEYCODES::KC_NUMPAD7:
				return 79;
			case E_KEYCODES::KC_NUMPAD8:
				return 80;
			case E_KEYCODES::KC_NUMPAD9:
				return 81;
			case E_KEYCODES::KC_A:
				return 38;
			case E_KEYCODES::KC_B:
				return 56;
			case E_KEYCODES::KC_C:
				return 54;
			case E_KEYCODES::KC_D:
				return 40;
			case E_KEYCODES::KC_E:
				return 26;
			case E_KEYCODES::KC_F:
				return 41;
			case E_KEYCODES::KC_G:
				return 42;
			case E_KEYCODES::KC_H:
				return 43;
			case E_KEYCODES::KC_I:
				return 31;
			case E_KEYCODES::KC_J:
				return 44;
			case E_KEYCODES::KC_K:
				return 45;
			case E_KEYCODES::KC_L:
			 	return 46;
			case E_KEYCODES::KC_M:
				return 58;
			case E_KEYCODES::KC_N:
				return 57;
			case E_KEYCODES::KC_O:
				return 32;
			case E_KEYCODES::KC_P:
				return 33;
			case E_KEYCODES::KC_Q:
				return 24;
			case E_KEYCODES::KC_R:
				return 27;
			case E_KEYCODES::KC_S:
				return 39;
			case E_KEYCODES::KC_T:
				return 28;
			case E_KEYCODES::KC_U:
				return 30;
			case E_KEYCODES::KC_V:
				return 55;
			case E_KEYCODES::KC_W:
				return 25;
			case E_KEYCODES::KC_X:
				return 53;
			case E_KEYCODES::KC_Y:
				return 29;
			case E_KEYCODES::KC_Z:
				return 52;
			case E_KEYCODES::KC_F1:
				return 67;
			case E_KEYCODES::KC_F2:
				return 68;
			case E_KEYCODES::KC_F3:
				return 69;
			case E_KEYCODES::KC_F4:
				return 70;
			case E_KEYCODES::KC_F5:
				return 71;
			case E_KEYCODES::KC_F6:
				return 72;
			case E_KEYCODES::KC_F7:
				return 73;
			case E_KEYCODES::KC_F8:
				return 74;
			case E_KEYCODES::KC_F9:
				return 75;
			case E_KEYCODES::KC_F10:
				return 76;
			case E_KEYCODES::KC_F11:
				return 95;
			case E_KEYCODES::KC_F12:
				return 96;
			// case E_KEYCODES::KC_F13:
			// 	return DIK_F13;
			// case E_KEYCODES::KC_F14:
			// 	return DIK_F14;
			// case E_KEYCODES::KC_F15:
			// 	return DIK_F15;
			case E_KEYCODES::KC_SPACE:
				return 65;
			case E_KEYCODES::KC_LSHIFT:
				return 50;
			case E_KEYCODES::KC_RSHIFT:
				return 62;
			case E_KEYCODES::KC_LALT:
				return 64;
			case E_KEYCODES::KC_RALT:
				return 108;
			case E_KEYCODES::KC_CAPSLOCK:
				return 66;
			case E_KEYCODES::KC_TAB:
				return 23;
			case E_KEYCODES::KC_RETURN:
				return 36;
			case E_KEYCODES::KC_LCONTROL:
				return 37;
			case E_KEYCODES::KC_RCONTROL:
				return 105;
			case E_KEYCODES::KC_RIGHT:
				return 114;
			case E_KEYCODES::KC_UP:
				return 111;
			case E_KEYCODES::KC_LEFT:
				return 113;
			case E_KEYCODES::KC_DOWN:
				return 116;
			case E_KEYCODES::KC_HOME:
				return 110;
 			// case E_KEYCODES::KC_END:
			// 	return 111;
			case E_KEYCODES::KC_PAGEUP:
				return 112;
 			case E_KEYCODES::KC_PAGEDOWN:
				return 117;
			case E_KEYCODES::KC_PAUSE:
				return 127;
			case E_KEYCODES::KC_BREAK:
				return 37;
			case E_KEYCODES::KC_BACKSPACE:
				return 22;
			case E_KEYCODES::KC_DELETE:
				return 119;
			case E_KEYCODES::KC_INSERT:
				return 118;
			// case E_KEYCODES::KC_LSYS:
			// 	return DIK_LWIN;
 			// case E_KEYCODES::KC_RSYS:
			// 	return DIK_RWIN;
			// /*case E_KEYCODES::KC_EXCLAIM:
			// 	return DIK_;
 			// case E_KEYCODES::KC_AT:
			// 	return DIK_AT;
 			// case E_KEYCODES::KC_DOUBLE_QUOTE:
			// 	return DIK_DOUBLE;
 			// case E_KEYCODES::KC_HASH:
			// 	return DIK_HASH;
 			// case E_KEYCODES::KC_DOLLAR:
			// 	return DIK_DOLLAR;
 			// case E_KEYCODES::KC_PERCENT:
			// 	return DIK_PERCENT;
 			// case E_KEYCODES::KC_AMPERSAND:
			// 	return DIK_;
			// case E_KEYCODES::KC_QUOTE:
			// 	return DIK_;
 			// case E_KEYCODES::KC_LPAREN:
			// 	return DIK_LPAREN;
 			// case E_KEYCODES::KC_RPAREN:
			// 	return DIK_;
 			// case E_KEYCODES::KC_ASTERISK:
			// 	return DIK_;*/
 			case E_KEYCODES::KC_PLUS:
			case E_KEYCODES::KC_EQUALS:
				return 21;
 			case E_KEYCODES::KC_MINUS:
				return 20;
			case E_KEYCODES::KC_COMMA:
				return 59;
 			case E_KEYCODES::KC_PERIOD:
				return 60;
 			case E_KEYCODES::KC_SLASH:
				return 61;
 			case E_KEYCODES::KC_BACKSLASH:
			case E_KEYCODES::KC_PIPE:
				return 51;
 			case E_KEYCODES::KC_COLON:
 			case E_KEYCODES::KC_SEMICOLON:
				return 47;
 			// /*case E_KEYCODES::KC_LESS:
			// 	return DIK_LESS;
			// case E_KEYCODES::KC_GREATER:
			// 	return DIK_GREATER;
 			// case E_KEYCODES::KC_QUESTION:
			// 	return DIK_;*/
 			// case E_KEYCODES::KC_LBRACKET:
			// 	return DIK_LBRACKET;
 			// case E_KEYCODES::KC_RBRACKET:
			// 	return DIK_RBRACKET;
 			// /*case E_KEYCODES::KC_CARET:
			// 	return DIK_CARET;
 			// case E_KEYCODES::KC_UNDERSCORE:
			// 	return DIK_UNDERSCORE;
			// case E_KEYCODES::KC_LCURLY_BRACKET:
			// 	return DIK_;
 			// case E_KEYCODES::KC_RCURLY_BRACKET:
			// 	return DIK_;*/
 			case E_KEYCODES::KC_TILDE:
			case E_KEYCODES::KC_BACK_QUOTE:
				return 49;
			case E_KEYCODES::KC_NUMLOCK:
				return 77;
			case E_KEYCODES::KC_SCROLL_LOCK:
				return 78;
			case E_KEYCODES::KC_PRINT:
			case E_KEYCODES::KC_SYSREQ:
				return 2;
		}

		return 0;
	}
			
	bool CUnixInputContext::_isKeyPressed(const C8 keysVector[32], U8 keycode)
	{
		return ((keysVector[keycode / 8] >> (keycode % 8)) & 0x1);
	}
	
	bool CUnixInputContext::_isButtonPressed(U32 buttonsMask, U8 buttonId)
	{
		return buttonsMask & (Button1Mask << buttonId);
	}


	TDE2_API IInputContext* CreateUnixInputContext(IWindowSystem* pWindowSystem, E_RESULT_CODE& result)
	{
		CUnixInputContext* pInputContextInstance = new (std::nothrow) CUnixInputContext();

		if (!pInputContextInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return pInputContextInstance;
		}

		result = pInputContextInstance->Init(pWindowSystem);

		if (result != RC_OK)
		{
			delete pInputContextInstance;

			pInputContextInstance = nullptr;
		}

		return dynamic_cast<IInputContext*>(pInputContextInstance);
	}
}

#endif