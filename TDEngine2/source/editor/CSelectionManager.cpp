#include "./../../include/editor/CSelectionManager.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CSelectionManager::CSelectionManager():
		CBaseObject()
	{
	}

	E_RESULT_CODE CSelectionManager::Init()
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CSelectionManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_OK;
		}

		mIsInitialized = false;
		delete this;

		return RC_OK;
	}


	TDE2_API ISelectionManager* CreateSelectionManager(E_RESULT_CODE& result)
	{
		CSelectionManager* pSelectionManagerInstance = new (std::nothrow) CSelectionManager();

		if (!pSelectionManagerInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pSelectionManagerInstance->Init();

		if (result != RC_OK)
		{
			delete pSelectionManagerInstance;

			pSelectionManagerInstance = nullptr;
		}

		return dynamic_cast<ISelectionManager*>(pSelectionManagerInstance);
	}
}

#endif