#include "./../include/CKeyboard.h"
#include <core/IInputContext.h>
#include "./../include/CDirectInputMappings.h"
#include <cstring>


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	CKeyboard::CKeyboard():
		CBaseInputDevice()
	{
	}

	E_RESULT_CODE CKeyboard::_createInternalHandlers(const TInternalInputData& params)
	{
		IDirectInput8* pInput = params.mpInput;

		if (FAILED(pInput->CreateDevice(GUID_SysKeyboard, &mpInputDevice, nullptr)))
		{
			return RC_FAIL;
		}

		/// \todo Replace DISCL_EXCLUSIVE | DISCL_FOREGROUND with a proper set up
		if (FAILED(mpInputDevice->SetCooperativeLevel(params.mWindowHandler, DISCL_EXCLUSIVE | DISCL_FOREGROUND)))
		{
			return RC_FAIL;
		}

		if (FAILED(mpInputDevice->SetDataFormat(&c_dfDIKeyboard)))
		{
			return RC_FAIL;
		}
		
		memset(mKeyboardState, 0, sizeof(U8) * mKeysCount);
		
		return RC_OK;
	}

	E_RESULT_CODE CKeyboard::Update()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		memcpy(mPrevKeyboardState, mKeyboardState, sizeof(U8) * mKeysCount);

		HRESULT internalResult = S_OK;

		if (FAILED(internalResult = mpInputDevice->GetDeviceState(mKeysCount, mKeyboardState)))
		{
			if (internalResult == DIERR_INPUTLOST)
			{
				return _acquireDevice(); /// reacquire the device if an access to it has been lost
			}

			return RC_FAIL;
		}

		return RC_OK;
	}

	bool CKeyboard::IsKeyPressed(E_KEYCODES keyCode)
	{
		U16 internalKeyCode = CDirectInputMappings::GetKeyCode(keyCode);

		if (internalKeyCode > mKeysCount)
		{
			return false;
		}

		return !(mPrevKeyboardState[internalKeyCode] & 0x80) && (mKeyboardState[internalKeyCode] & 0x80);
	}

	bool CKeyboard::IsKey(E_KEYCODES keyCode)
	{
		U16 internalKeyCode = CDirectInputMappings::GetKeyCode(keyCode);

		if (internalKeyCode > mKeysCount)
		{
			return false;
		}

		return mKeyboardState[internalKeyCode] & 0x80;
	}

	bool CKeyboard::IsKeyUnpressed(E_KEYCODES keyCode)
	{
		U16 internalKeyCode = CDirectInputMappings::GetKeyCode(keyCode);

		if (internalKeyCode > mKeysCount)
		{
			return false;
		}

		return (mPrevKeyboardState[internalKeyCode] & 0x80) && !(mKeyboardState[internalKeyCode] & 0x80);
	}


	TDE2_API IInputDevice* CreateKeyboardDevice(IInputContext* pInputContext, E_RESULT_CODE& result)
	{
		CKeyboard* pKeyboardInstance = new (std::nothrow) CKeyboard();

		if (!pKeyboardInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pKeyboardInstance->Init(pInputContext);

		if (result != RC_OK)
		{
			delete pKeyboardInstance;

			pKeyboardInstance = nullptr;
		}

		return dynamic_cast<IInputDevice*>(pKeyboardInstance);
	}
}

#endif