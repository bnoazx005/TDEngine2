#include "../include/CWindowsInputContext.h"
#include <core/IWindowSystem.h>
#include "../include/CKeyboard.h"
#include "../include/CMouse.h"
#include "../include/CGamepad.h"


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	CWindowsInputContext::CWindowsInputContext():
		CBaseObject()
	{
	}

	E_RESULT_CODE CWindowsInputContext::Init(TPtr<IWindowSystem> pWindowSystem)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = _createInputInternalHandler(pWindowSystem->GetInternalData().mWindowInstanceHandler);

		if (result != RC_OK)
		{
			return result;
		}
				
		mInternalData.mpInput        = mpInput;
		mInternalData.mWindowHandler = pWindowSystem->GetInternalData().mWindowHandler;

		mpKeyboardDevice = TPtr<IKeyboard>(dynamic_cast<IKeyboard*>(CreateKeyboardDevice(this, result)));

		if (result != RC_OK)
		{
			return result;
		}

		mpMouseDevice = TPtr<IMouse>(dynamic_cast<IMouse*>(CreateMouseDevice(this, result)));

		if (result != RC_OK)
		{
			return result;
		}

		for (U8 i = 0; i < mMaxNumOfGamepads; ++i)
		{
			mpGamepads[i] = TPtr<IGamepad>(dynamic_cast<IGamepad*>(CreateGamepadDevice(this, i, result)));

			if (result != RC_OK)
			{
				return result;
			}
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CWindowsInputContext::_onFreeInternal()
	{
		return _releaseInternalHandler();
	}

	E_RESULT_CODE CWindowsInputContext::Update()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mpKeyboardDevice->Update();

		mpMouseDevice->Update();

		for (U8 i = 0; i < mMaxNumOfGamepads; ++i)
		{
			if (auto pCurrGamepad = mpGamepads[i])
			{
				pCurrGamepad->Update();
			}
		}

		return RC_OK;
	}

	bool CWindowsInputContext::IsKeyPressed(E_KEYCODES keyCode)
	{
		return mpKeyboardDevice->IsKeyPressed(keyCode);
	}

	bool CWindowsInputContext::IsKey(E_KEYCODES keyCode)
	{
		return mpKeyboardDevice->IsKey(keyCode);
	}

	bool CWindowsInputContext::IsKeyUnpressed(E_KEYCODES keyCode)
	{
		return mpKeyboardDevice->IsKeyUnpressed(keyCode);
	}

	bool CWindowsInputContext::IsMouseButtonPressed(U8 button)
	{
		return mpMouseDevice->IsButtonPressed(button);
	}

	bool CWindowsInputContext::IsMouseButton(U8 button)
	{
		return mpMouseDevice->IsButton(button);
	}

	bool CWindowsInputContext::IsMouseButtonUnpressed(U8 button)
	{
		return mpMouseDevice->IsButtonUnpressed(button);
	}

	TVector3 CWindowsInputContext::GetMousePosition() const
	{
		return mpMouseDevice->GetMousePosition();
	}

	TVector2 CWindowsInputContext::GetNormalizedMousePosition() const
	{
		return mpMouseDevice->GetNormalizedMousePosition();
	}

	TVector3 CWindowsInputContext::GetMouseShiftVec() const
	{
		return mpMouseDevice->GetMouseShiftVec();
	}

	E_ENGINE_SUBSYSTEM_TYPE CWindowsInputContext::GetType() const
	{
		return EST_INPUT_CONTEXT;
	}

	const TInternalInputData& CWindowsInputContext::GetInternalHandler() const
	{
		return mInternalData;
	}

	TPtr<IGamepad> CWindowsInputContext::GetGamepad(U8 gamepadId) const
	{
		if (gamepadId >= mMaxNumOfGamepads)
		{
			return TPtr<IGamepad>(nullptr);
		}

		return mpGamepads[gamepadId];
	}

	E_RESULT_CODE CWindowsInputContext::_createInputInternalHandler(HINSTANCE windowHandler)
	{
		HRESULT internalResult = DirectInput8Create(windowHandler, DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(&mpInput), nullptr);

		if (FAILED(internalResult))
		{
			return RC_FAIL; /// \todo implement a mapping from HRESULT -> E_RESULT_CODE
		}

		return RC_OK;
	}

	E_RESULT_CODE CWindowsInputContext::_releaseInternalHandler()
	{
		if (mpInput && FAILED(mpInput->Release()))
		{
			return RC_FAIL;
		}

		return RC_OK;
	}


	TDE2_API IInputContext* CreateWindowsInputContext(TPtr<IWindowSystem> pWindowSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IInputContext, CWindowsInputContext, result, pWindowSystem);
	}
}

#endif