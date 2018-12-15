#include "./../include/CDirectInputContext.h"
#include <core/IWindowSystem.h>
#include "./../include/CKeyboard.h"
#include "./../include/CMouse.h"


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	CDirectInputContext::CDirectInputContext():
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CDirectInputContext::Init(IWindowSystem* pWindowSystem)
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

		mpKeyboardDevice = dynamic_cast<IKeyboard*>(CreateKeyboardDevice(this, result));

		if (result != RC_OK)
		{
			return result;
		}

		mpMouseDevice = dynamic_cast<IMouse*>(CreateMouseDevice(this, result));

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CDirectInputContext::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = _releaseInternalHandler();

		if (result != RC_OK)
		{
			return result;
		}

		if ((result = mpKeyboardDevice->Free()) != RC_OK)
		{
			return result;
		}

		if ((result = mpMouseDevice->Free()) != RC_OK)
		{
			return result;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CDirectInputContext::Update()
	{
		mpKeyboardDevice->Update();

		mpMouseDevice->Update();

		return RC_OK;
	}

	bool CDirectInputContext::IsKeyPressed(E_KEYCODES keyCode)
	{
		return mpKeyboardDevice->IsKeyPressed(keyCode);
	}

	bool CDirectInputContext::IsKey(E_KEYCODES keyCode)
	{
		return mpKeyboardDevice->IsKey(keyCode);
	}

	bool CDirectInputContext::IsKeyUnpressed(E_KEYCODES keyCode)
	{
		return mpKeyboardDevice->IsKeyUnpressed(keyCode);
	}

	bool CDirectInputContext::IsMouseButtonPressed(U8 button)
	{
		return mpMouseDevice->IsButtonPressed(button);
	}

	bool CDirectInputContext::IsMouseButton(U8 button)
	{
		return mpMouseDevice->IsButton(button);
	}

	bool CDirectInputContext::IsMouseButtonUnpressed(U8 button)
	{
		return mpMouseDevice->IsButtonUnpressed(button);
	}

	TVector3 CDirectInputContext::GetMousePosition() const
	{
		return mpMouseDevice->GetMousePosition();
	}

	E_ENGINE_SUBSYSTEM_TYPE CDirectInputContext::GetType() const
	{
		return EST_INPUT_CONTEXT;
	}

	const TInternalInputData& CDirectInputContext::GetInternalHandler() const
	{
		return mInternalData;
	}

	E_RESULT_CODE CDirectInputContext::_createInputInternalHandler(HINSTANCE windowHandler)
	{
		HRESULT internalResult = DirectInput8Create(windowHandler, DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(&mpInput), nullptr);

		if (FAILED(internalResult))
		{
			return RC_FAIL; /// \todo implement a mapping from HRESULT -> E_RESULT_CODE
		}

		return RC_OK;
	}

	E_RESULT_CODE CDirectInputContext::_releaseInternalHandler()
	{
		if (mpInput && FAILED(mpInput->Release()))
		{
			return RC_FAIL;
		}

		return RC_OK;
	}


	TDE2_API IInputContext* CreateDirectInputContext(IWindowSystem* pWindowSystem, E_RESULT_CODE& result)
	{
		CDirectInputContext* pInputContextInstance = new (std::nothrow) CDirectInputContext();

		if (!pInputContextInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
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