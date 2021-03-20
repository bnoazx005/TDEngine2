#include "../include/CParticleEditorWindow.h"


namespace TDEngine2
{
	CParticleEditorWindow::CParticleEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CParticleEditorWindow::Init()
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		mIsInitialized = true;
		mIsVisible = true;

		return RC_OK;
	}

	E_RESULT_CODE CParticleEditorWindow::Free()
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

	void CParticleEditorWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(350.0f, 200.0f),
			TVector2(1000.0f, 550.0f),
		};

		if (mpImGUIContext->BeginWindow("Particle Editor", isEnabled, params))
		{
			_drawToolbar();

		}

		mpImGUIContext->EndWindow();

		mIsVisible = isEnabled;
	}

	void CParticleEditorWindow::_drawToolbar()
	{
		if (mpImGUIContext->BeginChildWindow("Toolbar", TVector2(mpImGUIContext->GetWindowWidth(), 15.0f)))
		{
			

			mpImGUIContext->EndChildWindow();
		}
	}


	TDE2_API IEditorWindow* CreateParticleEditorWindow(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CParticleEditorWindow, result);
	}
}