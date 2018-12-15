#include "./../include/CMouse.h"
#include <core/IInputContext.h>
#include <cstring>


#if defined (TDE2_USE_WIN32PLATFORM)

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
			if (internalResult == DIERR_INPUTLOST)
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
		return {};
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