#include "../include/CConfigWindow.h"
#include <tuple>
#include <vector>
#include <string>


namespace TDEngine2
{
	CConfigWindow::CConfigWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CConfigWindow::Init(IResourceManager* pResourceManager, IInputContext* pInputContext, IWindowSystem* pWindowSystem)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pResourceManager || !pResourceManager || !pWindowSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;
		mpInputContext = dynamic_cast<IDesktopInputContext*>(pInputContext);
		mpWindowSystem = pWindowSystem;

		mIsInitialized = true;
		mIsVisible = true;

		return RC_OK;
	}

	E_RESULT_CODE CConfigWindow::Free()
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

	void CConfigWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(300.0f, 150.0f),
			TVector2(300.0f, 1000.0f),
		};

		if (mpImGUIContext->BeginWindow("Settings", isEnabled, params))
		{
		}

		mpImGUIContext->EndWindow();

		mIsVisible = isEnabled;
	}


	TDE2_API IEditorWindow* CreateConfigWindow(IResourceManager* pResourceManager, IInputContext* pInputContext, IWindowSystem* pWindowSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CConfigWindow, result, pResourceManager, pInputContext, pWindowSystem);
	}
}