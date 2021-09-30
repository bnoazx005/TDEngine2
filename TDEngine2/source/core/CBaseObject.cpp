#include "../../include/core/CBaseObject.h"


namespace TDEngine2
{
	CBaseObject::CBaseObject() :
		mIsInitialized(false), mRefCounter(1)
	{
	}

	void CBaseObject::AddRef()
	{
		++mRefCounter;
	}

	E_RESULT_CODE CBaseObject::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		--mRefCounter;

		if (!mRefCounter)
		{
			result = _onFreeInternal();

			mIsInitialized = false;
			delete this;
		}

		return RC_OK;
	}

	U32 CBaseObject::GetRefCount() const
	{
		return mRefCounter;
	}
	
	E_RESULT_CODE CBaseObject::_onFreeInternal()
	{
		return RC_OK;
	}
}