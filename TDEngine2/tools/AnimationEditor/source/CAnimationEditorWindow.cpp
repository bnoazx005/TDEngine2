#include "../include/CAnimationEditorWindow.h"


namespace TDEngine2
{
	CAnimationEditorWindow::CAnimationEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CAnimationEditorWindow::Init()
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		mIsInitialized = true;
		mIsVisible = true;

		return RC_OK;
	}

	E_RESULT_CODE CAnimationEditorWindow::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		--mRefCounter;

		if (!mRefCounter)
		{
			mIsInitialized = false;
			delete this;
		}

		return RC_OK;
	}

	void CAnimationEditorWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(450.0f, 200.0f),
			TVector2(1000.0f, 550.0f),
		};

		if (mpImGUIContext->BeginWindow("Animation Editor", isEnabled, params))
		{
			_drawToolbar();

			mpImGUIContext->Label("TTTT");
		}

		mpImGUIContext->EndWindow();

		mIsVisible = isEnabled;
	}

	void CAnimationEditorWindow::_drawToolbar()
	{
		if (mpImGUIContext->BeginChildWindow("Toolbar", TVector2(mpImGUIContext->GetWindowWidth(), 15.0f)))
		{
			mpImGUIContext->BeginHorizontal();

			mpImGUIContext->Button("##Play", TVector2(20.0f, 15.0f));
			mpImGUIContext->Button("##BackStep", TVector2(20.0f, 15.0f));
			mpImGUIContext->Button("##ForwardStep", TVector2(20.0f, 15.0f));
			mpImGUIContext->Button("##Record", TVector2(20.0f, 15.0f));

			mpImGUIContext->EndHorizontal();

			mpImGUIContext->EndChildWindow();
		}
	}


	TDE2_API IEditorWindow* CreateAnimationEditorWindow(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CAnimationEditorWindow, result);
	}
}