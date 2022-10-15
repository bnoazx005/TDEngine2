#include "../../include/editor/CStatsViewerWindow.h"
#include "../../include/core/IImGUIContext.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CStatsViewerWindow::CStatsViewerWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CStatsViewerWindow::Init()
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	void CStatsViewerWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(500.0f, 300.0f),
			TVector2(1e+30f, 1e+30f),
			false, 
			true,
			true,
			true
		};

		if (mpImGUIContext->BeginWindow("Stats Viewer", isEnabled, params))
		{
			mpImGUIContext->SetCursorScreenPos(TVector2(mpImGUIContext->GetWindowWidth() * 0.8f, 10.0f));

			if (mpImGUIContext->BeginChildWindow("##Stats", TVector2(400.0f, 600.0f)))
			{
				const TVector2 initialPos = mpImGUIContext->GetCursorScreenPos();
				const TVector2 vOffset(0.0f, 15.0f), hOffset(120.0f, 0.0f);

				mpImGUIContext->DrawText(initialPos, TColorUtils::mWhite, "Stats: ");

				mpImGUIContext->DrawText(initialPos + vOffset, TColorUtils::mWhite, "FPS: ");
				mpImGUIContext->DrawText(initialPos + vOffset + hOffset, TColorUtils::mWhite, std::to_string(static_cast<I32>(1.0f / CMathUtils::Max(1e-3f, mCurrDelta))));
			
				mpImGUIContext->DrawText(initialPos + 2.0f * vOffset, TColorUtils::mWhite, "Frame Time (ms): ");
				mpImGUIContext->DrawText(initialPos + 2.0f * vOffset + hOffset, TColorUtils::mWhite, std::to_string(mCurrDelta * 1000.0f));


				mpImGUIContext->EndChildWindow();
			}
		}

		mpImGUIContext->EndWindow();

		mIsVisible = isEnabled;
	}


	TDE2_API IEditorWindow* CreateStatsViewerWindow(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CStatsViewerWindow, result);
	}
}

#endif