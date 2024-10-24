#include "./../include/CMouse.h"
#include <core/IInputContext.h>
#include <math/MathUtils.h>
#include <cstring>


#if defined (TDE2_USE_WINPLATFORM)

namespace TDEngine2
{
	CMouse::CMouse() :
		CBaseInputDevice()
	{
	}


	E_RESULT_CODE CMouse::_createInternalHandlers(const TInternalInputData& params)
	{
		IDirectInput8* pInput = params.mpInput;

		if (FAILED(pInput->CreateDevice(GUID_SysMouse, &mpInputDevice, nullptr)))
		{
			return RC_FAIL;
		}

		/// \todo Replace DISCL_EXCLUSIVE | DISCL_FOREGROUND with a proper set up
		if (FAILED(mpInputDevice->SetCooperativeLevel(params.mWindowHandler, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND)))
		{
			return RC_FAIL;
		}

		if (FAILED(mpInputDevice->SetDataFormat(&c_dfDIMouse2)))
		{
			return RC_FAIL;
		}

		memset(&mCurrMouseState, 0, sizeof(DIMOUSESTATE2));

		return RC_OK;
	}

	E_RESULT_CODE CMouse::Update()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		memcpy(&mPrevMouseState, &mCurrMouseState, sizeof(DIMOUSESTATE2));

		HRESULT internalResult = S_OK;

		if (FAILED(internalResult = mpInputDevice->GetDeviceState(sizeof(DIMOUSESTATE2), &mCurrMouseState)))
		{
			if (internalResult == DIERR_INPUTLOST || internalResult == DIERR_NOTACQUIRED)
			{
				return _acquireDevice(); /// reacquire the device if an access to it has been lost
			}

			return RC_FAIL;
		}
		
		return RC_OK;
	}

	bool CMouse::IsButtonPressed(U8 button)
	{
		if (button >= 8) /// \note the mice with more than 8 buttons are not supported
		{
			return false;
		}

		return !(mPrevMouseState.rgbButtons[button] & 0x80) && (mCurrMouseState.rgbButtons[button] & 0x80);
	}

	bool CMouse::IsButton(U8 button)
	{
		if (button >= 8) /// \note the mice with more than 8 buttons are not supported
		{
			return false;
		}

		return mCurrMouseState.rgbButtons[button] & 0x80;
	}

	bool CMouse::IsButtonUnpressed(U8 button)
	{
		if (button >= 8) /// \note the mice with more than 8 buttons are not supported
		{
			return false;
		}

		return (mPrevMouseState.rgbButtons[button] & 0x80) && !(mCurrMouseState.rgbButtons[button] & 0x80);
	}

	TVector3 CMouse::GetMousePosition() const
	{
		POINT cursorPos;

		GetCursorPos(&cursorPos);

		HWND hwnd = mpWinInputContext->GetInternalHandler().mWindowHandler;

		ScreenToClient(hwnd, &cursorPos); /// convert screen coordinates into client area's ones

		RECT windowBounds;

		GetClientRect(hwnd, &windowBounds);
		
		F32 x = static_cast<F32>(cursorPos.x);
		
		/// convert from space where the origin is placed at left top corner to a space with the origin at left bottom corner
		F32 y = static_cast<F32>(windowBounds.bottom - cursorPos.y);

		return { x, y, 0.0f };
	}

	TVector2 CMouse::GetNormalizedMousePosition() const
	{
		TVector3 pos = GetMousePosition();

		RECT windowBounds;

		HWND hwnd = mpWinInputContext->GetInternalHandler().mWindowHandler;
		GetClientRect(hwnd, &windowBounds);		

		F32 width  = static_cast<F32>(windowBounds.right);
		F32 height = static_cast<F32>(windowBounds.bottom);

		return { CMathUtils::Clamp(-1.0f, 1.0f, 2.0f * pos.x / width - 1.0f), CMathUtils::Clamp(-1.0f, 1.0f, 2.0f * pos.y / height - 1.0f) };
	}

	TVector3 CMouse::GetMouseShiftVec() const
	{
		return { static_cast<F32>(mCurrMouseState.lX), static_cast<F32>(mCurrMouseState.lY), static_cast<F32>(mCurrMouseState.lZ) };
	}
	

	TDE2_API IInputDevice* CreateMouseDevice(IInputContext* pInputContext, E_RESULT_CODE& result)
	{
		CMouse* pMouseInstance = new (std::nothrow) CMouse();

		if (!pMouseInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pMouseInstance->Init(pInputContext);

		if (result != RC_OK)
		{
			delete pMouseInstance;

			pMouseInstance = nullptr;
		}
		
		return dynamic_cast<IInputDevice*>(pMouseInstance);
	}
}

#endif