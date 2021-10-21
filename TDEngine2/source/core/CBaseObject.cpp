#include "../../include/core/CBaseObject.h"
#if TDE2_EDITORS_ENABLED
#include "../../include/editor/CMemoryProfiler.h"
#endif


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
		if (!mRefCounter)
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
#if TDE2_EDITORS_ENABLED
		CMemoryProfiler::Get()->UnregisterBaseObject();
#endif

		return RC_OK;
	}

#if TDE2_EDITORS_ENABLED
	
	void CBaseObject::RegisterObjectInProfiler()
	{
		CMemoryProfiler::Get()->RegisterBaseObject();
	}

#endif
}