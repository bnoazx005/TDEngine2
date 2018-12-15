#include "./../include/CBaseInputDevice.h"
#include <core/IInputContext.h>


#if defined (TDE2_USE_WIN32PLATFORM)

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

		CDirectInputContext* pDirectInputContext = dynamic_cast<CDirectInputContext*>(pInputContext);

		if (!pDirectInputContext)
		{
			return RC_INVALID_ARGS;
		}
				
		E_RESULT_CODE result = _createInternalHandlers(pDirectInputContext->GetInternalHandler());

		if (result != RC_OK)
		{
			return result;
		}

		result = _acquireDevice();

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBaseInputDevice::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (mpInputDevice &&
			FAILED(mpInputDevice->Unacquire()) &&
			FAILED(mpInputDevice->Release()))
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CBaseInputDevice::_acquireDevice()
	{
		if (FAILED(mpInputDevice->Acquire()))
		{
			return RC_FAIL;
		}

		return RC_OK;
	}
}

#endif