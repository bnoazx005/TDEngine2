#include "./../../include/editor/CLevelEditorWindow.h"
#include "./../../include/core/IImGUIContext.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CLevelEditorWindow::CLevelEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CLevelEditorWindow::Init()
	{
		if (mIsInitialized)
		{
			return RC_OK;
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
	}


	TDE2_API IEditorWindow* CreateLevelEditorWindow(E_RESULT_CODE& result)
	{
		CLevelEditorWindow* pEditorInstance = new (std::nothrow) CLevelEditorWindow();

		if (!pEditorInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pEditorInstance->Init();

		if (result != RC_OK)
		{
			delete pEditorInstance;

			pEditorInstance = nullptr;
		}

		return dynamic_cast<IEditorWindow*>(pEditorInstance);
	}
}

#endif