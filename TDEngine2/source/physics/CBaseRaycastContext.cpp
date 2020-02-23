#include "./../../include/physics/CBaseRaycastContext.h"


namespace TDEngine2
{
	CBaseRaycastContext::CBaseRaycastContext() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CBaseRaycastContext::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBaseRaycastContext::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;
		
		delete this;

		return RC_OK;
	}


	TDE2_API IRaycastContext* CreateBaseRaycastContext(E_RESULT_CODE& result)
	{
		CBaseRaycastContext* pRaycastContextInstance = new (std::nothrow) CBaseRaycastContext();

		if (!pRaycastContextInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pRaycastContextInstance->Init();

		if (result != RC_OK)
		{
			delete pRaycastContextInstance;

			pRaycastContextInstance = nullptr;
		}

		return dynamic_cast<IRaycastContext*>(pRaycastContextInstance);
	}
}