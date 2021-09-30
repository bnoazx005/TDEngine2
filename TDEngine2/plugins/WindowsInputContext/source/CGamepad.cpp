#include "./../include/CGamepad.h"
#include <core/IInputContext.h>
#include "./../include/CDirectInputMappings.h"
#include <cstring>
#include <climits>


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	CGamepad::CGamepad() :
		CBaseObject()
	{
	}
	
	E_RESULT_CODE CGamepad::Init(IInputContext* pInputContext)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		/// retrieve the first free gamepad's id
		U16 gamepadId = (std::numeric_limits<U16>::max)();

		memset(&mCurrGamepadState, 0, sizeof(XINPUT_STATE));

		for (U16 i = 0; i < XUSER_MAX_COUNT; ++i)
		{
			if (XInputGetState(i, &mCurrGamepadState) == ERROR_SUCCESS)
			{
				gamepadId = i;

				break;
			}
		}

		if (gamepadId >= XUSER_MAX_COUNT)
		{
			return RC_FAIL;
		}

		mGamepadId = gamepadId;
		
		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CGamepad::Init(IInputContext* pInputContext, U8 gamepadId)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (gamepadId >= XUSER_MAX_COUNT)
		{
			return RC_INVALID_ARGS;
		}
		
		mGamepadId = gamepadId;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CGamepad::Update()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}
		
		memcpy(&mPrevGamepadState, &mCurrGamepadState, sizeof(XINPUT_STATE));

		if (XInputGetState(mGamepadId, &mCurrGamepadState) != ERROR_SUCCESS)
		{
			memset(&mCurrGamepadState, 0, sizeof(XINPUT_STATE));

			return RC_FAIL;
		}

		return RC_OK;
	}

	bool CGamepad::IsButtonPressed(E_GAMEPAD_BUTTONS button) const
	{
		U16 internalButtonMask = CDirectInputMappings::GetGamepadButton(button);

		return !(mPrevGamepadState.Gamepad.wButtons & internalButtonMask) &&
			   (mCurrGamepadState.Gamepad.wButtons & internalButtonMask);
	}

	bool CGamepad::IsButton(E_GAMEPAD_BUTTONS button) const
	{
		return mCurrGamepadState.Gamepad.wButtons & CDirectInputMappings::GetGamepadButton(button);
	}

	bool CGamepad::IsButtonUnpressed(E_GAMEPAD_BUTTONS button) const
	{
		U16 internalButtonMask = CDirectInputMappings::GetGamepadButton(button);

		return (mPrevGamepadState.Gamepad.wButtons & internalButtonMask) &&
			   !(mCurrGamepadState.Gamepad.wButtons & internalButtonMask);
	}

	F32 CGamepad::GetLTriggerValue() const
	{
		return mCurrGamepadState.Gamepad.bLeftTrigger / 255.0f;
	}

	F32 CGamepad::GetRTriggerValue() const
	{
		return mCurrGamepadState.Gamepad.bRightTrigger / 255.0f;
	}

	TVector2 CGamepad::GetLThumbShiftVec() const
	{
		return _filterStickRawData(mCurrGamepadState.Gamepad.sThumbLX, mCurrGamepadState.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
	}

	TVector2 CGamepad::GetRThumbShiftVec() const
	{
		return _filterStickRawData(mCurrGamepadState.Gamepad.sThumbRX, mCurrGamepadState.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
	}

	TVector2 CGamepad::_filterStickRawData(I16 x, I16 y, U16 deadzoneValue) const
	{
		TVector2 shiftVec(static_cast<F32>(x), static_cast<F32>(y));

		if (Dot(shiftVec, shiftVec) < deadzoneValue * deadzoneValue) // Dot(x, x) is a squared length of x, if the offset doesn't exceed the specified dead zone we reset it
		{
			return ZeroVector2;
		}

		/// compute percentage 
		F32 percentage = (Length(shiftVec) - deadzoneValue) / ((std::numeric_limits<U16>::max)() - deadzoneValue);

		return Normalize(shiftVec) * percentage;
	}
	

	TDE2_API IInputDevice* CreateGamepadDevice(IInputContext* pInputContext, U8 gamepadId, E_RESULT_CODE& result)
	{
		CGamepad* pGamepadInstance = new (std::nothrow) CGamepad();

		if (!pGamepadInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pGamepadInstance->Init(pInputContext, gamepadId);

		if (result != RC_OK)
		{
			delete pGamepadInstance;

			pGamepadInstance = nullptr;
		}

		return dynamic_cast<IInputDevice*>(pGamepadInstance);
	}
}

#endif