#include "./../../include/editor/CEditorsManager.h"
#include "./../../include/core/IImGUIContext.h"
#include "./../../include/core/IInputContext.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CEditorsManager::CEditorsManager():
		CBaseObject(), mIsVisible(false)
	{
	}

	E_RESULT_CODE CEditorsManager::Init(IInputContext* pInputContext, IImGUIContext* pImGUIContext)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pImGUIContext || !pInputContext)
		{
			return RC_INVALID_ARGS;
		}

		mpInputContext = dynamic_cast<IDesktopInputContext*>(pInputContext);
		mpImGUIContext = pImGUIContext;

		mIsVisible = false;

		if (!mpInputContext)
		{
			return RC_INVALID_ARGS;
		}

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
		if (mpInputContext->IsKeyPressed(E_KEYCODES::KC_TILDE))
		{
			mIsVisible = !mIsVisible;
		}

		return _showEditorWindows();
	}

	E_ENGINE_SUBSYSTEM_TYPE CEditorsManager::GetType() const
	{
		return EST_EDITORS_MANAGER;
	}

	E_RESULT_CODE CEditorsManager::_showEditorWindows()
	{
		if (!mIsVisible)
		{
			return RC_OK;
		}

		if (mpImGUIContext->BeginWindow("Development Menu", mIsVisible))
		{
			// \todo Draw all buttons here, next step is to add sub-menus based on states

			mpImGUIContext->EndWindow();
		}

		return RC_OK;
	}


	TDE2_API IEditorsManager* CreateEditorsManager(IInputContext* pInputContext, IImGUIContext* pImGUIContext, E_RESULT_CODE& result)
	{
		CEditorsManager* pEditorsManagerInstance = new (std::nothrow) CEditorsManager();

		if (!pEditorsManagerInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pEditorsManagerInstance->Init(pInputContext, pImGUIContext);

		if (result != RC_OK)
		{
			delete pEditorsManagerInstance;

			pEditorsManagerInstance = nullptr;
		}

		return dynamic_cast<IEditorsManager*>(pEditorsManagerInstance);
	}
}

#endif