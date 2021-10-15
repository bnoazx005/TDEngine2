#include "./../include/CBaseInputDevice.h"
#include <core/IInputContext.h>


#if defined (TDE2_USE_WINPLATFORM)

namespace TDEngine2
{
	CBaseInputDevice::CBaseInputDevice() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CBaseInputDevice::Init(IInputContext* pInputContext)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		CWindowsInputContext* pWinInputContext = dynamic_cast<CWindowsInputContext*>(pInputContext);

		if (!pWinInputContext)
		{
			return RC_INVALID_ARGS;
		}

		mpWinInputContext = pWinInputContext;
				
		E_RESULT_CODE result = _createInternalHandlers(pWinInputContext->GetInternalHandler());

		if (result != RC_OK)
		{
			return result;
		}

		_acquireDevice();

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBaseInputDevice::_onFreeInternal()
	{
		if (mpInputDevice &&
			FAILED(mpInputDevice->Unacquire()) &&
			FAILED(mpInputDevice->Release()))
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	E_RESULT_CODE CBaseInputDevice::_acquireDevice()
	{
		HRESULT internalResult = S_OK;

		if (FAILED(internalResult = mpInputDevice->Acquire()))
		{
			return RC_FAIL;
		}

		return RC_OK;
	}
}

#endif