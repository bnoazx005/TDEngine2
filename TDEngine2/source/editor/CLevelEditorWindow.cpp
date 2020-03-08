#include "./../../include/editor/CLevelEditorWindow.h"
#include "./../../include/editor/IEditorsManager.h"
#include "./../../include/editor/ISelectionManager.h"
#include "./../../include/core/IImGUIContext.h"
#include "./../../include/core/IInputContext.h"
#include "./../../include/utils/CFileLogger.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CLevelEditorWindow::CLevelEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CLevelEditorWindow::Init(IEditorsManager* pEditorsManager, IInputContext* pInputContext)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pEditorsManager || !pInputContext)
		{
			return RC_INVALID_ARGS;
		}

		mpEditorsManager   = pEditorsManager;
		mpInputContext     = dynamic_cast<IDesktopInputContext*>(pInputContext);
		mpSelectionManager = nullptr;

		if (!mpInputContext)
		{
			return RC_INVALID_ARGS;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CLevelEditorWindow::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;
		delete this;

		return RC_OK;
	}

	void CLevelEditorWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(150.0f, 400.0f),
			TVector2(150.0f, 400.0f),
		};

		if (mpImGUIContext->BeginWindow("Level Editor", isEnabled, params))
		{
		}

		mpImGUIContext->EndWindow();

		mIsVisible = isEnabled;

		_onHandleInput();
	}

	void CLevelEditorWindow::_onHandleInput()
	{
		ISelectionManager* pSelectionManager = _getSelectionManager();

		if (mpInputContext->IsMouseButtonPressed(0))
		{
			TVector3 mousePosition = mpInputContext->GetMousePosition();
			LOG_MESSAGE(CStringUtils::Format("[Level Editor] Picked object id : {0}", pSelectionManager->PickObject({ mousePosition.x, mousePosition.y })));
		}

	}

	ISelectionManager* CLevelEditorWindow::_getSelectionManager()
	{
		if (!mpSelectionManager)
		{
			mpSelectionManager = mpEditorsManager->GetSelectionManager();
		}

		return mpSelectionManager;
	}


	TDE2_API IEditorWindow* CreateLevelEditorWindow(IEditorsManager* pEditorsManager, IInputContext* pInputContext, E_RESULT_CODE& result)
	{
		CLevelEditorWindow* pEditorInstance = new (std::nothrow) CLevelEditorWindow();

		if (!pEditorInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pEditorInstance->Init(pEditorsManager, pInputContext);

		if (result != RC_OK)
		{
			delete pEditorInstance;

			pEditorInstance = nullptr;
		}

		return dynamic_cast<IEditorWindow*>(pEditorInstance);
	}
}

#endif