#include "../../include/editor/CSceneHierarchyWindow.h"
#include "../../include/core/IImGUIContext.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CSceneHierarchyEditorWindow::CSceneHierarchyEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CSceneHierarchyEditorWindow::Init()
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CSceneHierarchyEditorWindow::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;
		delete this;

		return RC_OK;
	}

	void CSceneHierarchyEditorWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(300.0f, 500.0f),
			TVector2(500.0f, 500.0f),
		};

		if (mpImGUIContext->BeginWindow("Scene Hierarchy", isEnabled, params))
		{
			
			mpImGUIContext->EndWindow();
		}

		mIsVisible = isEnabled;
	}


	TDE2_API IEditorWindow* CreateSceneHierarchyEditorWindow(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CSceneHierarchyEditorWindow, result);
	}
}

#endif