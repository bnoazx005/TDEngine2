#include "./../../include/graphics/CDebugUtility.h"
#include "./../../include/graphics/IGraphicsObjectManager.h"


namespace TDEngine2
{
	CDebugUtility::CDebugUtility():
		CBaseObject()
	{
	}

	E_RESULT_CODE CDebugUtility::Init(IGraphicsObjectManager* pGraphicsObjectManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsObjectManager)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsObjectManager = pGraphicsObjectManager;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CDebugUtility::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	void CDebugUtility::DrawLine(const TVector3& start, const TVector3& end, const TColor32F& color)
	{
		if (!mIsInitialized)
		{
			return;
		}

		TDE2_UNIMPLEMENTED();
	}


	TDE2_API IDebugUtility* CreateDebugUtility(IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		CDebugUtility* pDebugUtilityInstance = new (std::nothrow) CDebugUtility();

		if (!pDebugUtilityInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pDebugUtilityInstance->Init(pGraphicsObjectManager);

		if (result != RC_OK)
		{
			delete pDebugUtilityInstance;
			
			pDebugUtilityInstance = nullptr;
		}

		return pDebugUtilityInstance;
	}
}