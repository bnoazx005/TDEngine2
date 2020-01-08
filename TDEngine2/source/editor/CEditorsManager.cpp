#include "./../../include/editor/CEditorsManager.h"
#include "./../../include/core/IImGUIContext.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CEditorsManager::CEditorsManager():
		CBaseObject()
	{
	}

	E_RESULT_CODE CEditorsManager::Init(IImGUIContext* pImGUIContext)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pImGUIContext)
		{
			return RC_INVALID_ARGS;
		}

		mpImGUIContext = pImGUIContext;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CEditorsManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_OK;
		}

		mIsInitialized = false;
		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CEditorsManager::Update()
	{

		return RC_OK;
	}

	E_ENGINE_SUBSYSTEM_TYPE CEditorsManager::GetType() const
	{
		return EST_EDITORS_MANAGER;
	}


	TDE2_API IEditorsManager* CreateEditorsManager(IImGUIContext* pImGUIContext, E_RESULT_CODE& result)
	{
		CEditorsManager* pEditorsManagerInstance = new (std::nothrow) CEditorsManager();

		if (!pEditorsManagerInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pEditorsManagerInstance->Init(pImGUIContext);

		if (result != RC_OK)
		{
			delete pEditorsManagerInstance;

			pEditorsManagerInstance = nullptr;
		}

		return dynamic_cast<IEditorsManager*>(pEditorsManagerInstance);
	}
}

#endif