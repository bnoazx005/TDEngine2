#include "../../include/editor/CRenderTargetViewerWindow.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/core/IResourceManager.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CRenderTargetViewerEditorWindow::CRenderTargetViewerEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CRenderTargetViewerEditorWindow::Init(IResourceManager* pResourceManager)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CRenderTargetViewerEditorWindow::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;
		delete this;

		return RC_OK;
	}

	void CRenderTargetViewerEditorWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(250.0f, 250.0f),
			TVector2(500.0f, 500.0f),
		};

		if (mpImGUIContext->BeginWindow("Render Target Viewer", isEnabled, params))
		{
			

			// \todo Get list of all render targets
			// \todo Show selected texture

			mpImGUIContext->EndWindow();
		}

		mIsVisible = isEnabled;
	}


	TDE2_API IEditorWindow* CreateRenderTargetViewerEditorWindow(IResourceManager* pResourceManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CRenderTargetViewerEditorWindow, result, pResourceManager);
	}
}

#endif