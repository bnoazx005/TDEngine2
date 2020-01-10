#include "./../../include/editor/CProfilerEditorWindow.h"
#include "./../../include/editor/IProfiler.h"
#include "./../../include/core/IImGUIContext.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CProfilerEditorWindow::CProfilerEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CProfilerEditorWindow::Init(IProfiler* pProfiler)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pProfiler)
		{
			return RC_INVALID_ARGS;
		}

		mpProfiler = pProfiler;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CProfilerEditorWindow::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;
		delete this;

		return RC_OK;
	}

	void CProfilerEditorWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;
		
		if (mpImGUIContext->BeginWindow("Profiler", isEnabled))
		{
			// \todo

			mpImGUIContext->EndWindow();
		}

		mIsVisible = isEnabled;
	}


	TDE2_API IEditorWindow* CreateProfilerEditorWindow(IProfiler* pProfiler, E_RESULT_CODE& result)
	{
		CProfilerEditorWindow* pEditorInstance = new (std::nothrow) CProfilerEditorWindow();

		if (!pEditorInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pEditorInstance->Init(pProfiler);

		if (result != RC_OK)
		{
			delete pEditorInstance;

			pEditorInstance = nullptr;
		}

		return dynamic_cast<IEditorWindow*>(pEditorInstance);
	}
}

#endif