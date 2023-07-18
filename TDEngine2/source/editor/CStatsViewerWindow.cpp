#include "../../include/editor/CStatsViewerWindow.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/editor/CPerfProfiler.h"


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


	static TVector2 DrawTextLine(IImGUIContext* pImGUIContext, const TVector2& carretPos, F32 indent, F32 lineHeight, 
		const std::string& header = Wrench::StringUtils::GetEmptyStr(), const std::string& text = Wrench::StringUtils::GetEmptyStr())
	{
		static constexpr F32 horizontalSpace = 160.0f;

		if (!header.empty() && !text.empty())
		{
			pImGUIContext->DrawText(carretPos + TVector2(indent, 0.0f), TColorUtils::mWhite, header);
			pImGUIContext->DrawText(carretPos + TVector2(horizontalSpace, 0.0f), TColorUtils::mWhite, text);
		}

		return carretPos + TVector2(0.0f, lineHeight);
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
			mpImGUIContext->SetCursorScreenPos(TVector2(mpImGUIContext->GetWindowWidth() * 0.77f, 10.0f));

			if (mpImGUIContext->BeginChildWindow("##Stats", TVector2(400.0f, 600.0f)))
			{
				TVector2 currPos = mpImGUIContext->GetCursorScreenPos();
				const TVector2 vOffset(0.0f, 15.0f);

				mpImGUIContext->DrawText(currPos, TColorUtils::mWhite, "Stats: ");

				currPos = DrawTextLine(mpImGUIContext, currPos + vOffset, 0.0f, vOffset.y, "FPS: ", std::to_string(static_cast<I32>(1.0f / CMathUtils::Max(1e-3f, mCurrDeltaTime))));
				
				{
					currPos = DrawTextLine(mpImGUIContext, currPos, 0.0f, vOffset.y);
					currPos = DrawTextLine(mpImGUIContext, currPos, 0.0f, vOffset.y, "Frame Time (ms): ", std::to_string(mCurrDeltaTime * 1000.0f));

					currPos = DrawTextLine(mpImGUIContext, currPos, 20.0f, vOffset.y, "Update Time (ms): ",
						std::to_string(CPerfProfiler::Get()->GetAverageTimeByEventName(E_SPECIAL_PROFILE_EVENT::UPDATE) * 1000.0f));

					currPos = DrawTextLine(mpImGUIContext, currPos, 25.0f, vOffset.y, "World Update (ms): ",
						std::to_string(CPerfProfiler::Get()->GetAverageTimeByEventName(E_SPECIAL_PROFILE_EVENT::WORLD_UPDATE) * 1000.0f));

					currPos = DrawTextLine(mpImGUIContext, currPos, 25.0f, vOffset.y, "Audio Update (ms): ",
						std::to_string(CPerfProfiler::Get()->GetAverageTimeByEventName(E_SPECIAL_PROFILE_EVENT::AUDIO_UPDATE) * 1000.0f));

					currPos = DrawTextLine(mpImGUIContext, currPos, 20.0f, vOffset.y, "Render Time (ms): ",
						std::to_string(CPerfProfiler::Get()->GetAverageTimeByEventName(E_SPECIAL_PROFILE_EVENT::RENDER) * 1000.0f));

					currPos = DrawTextLine(mpImGUIContext, currPos, 20.0f, vOffset.y, "Present (ms): ",
						std::to_string(CPerfProfiler::Get()->GetAverageTimeByEventName(E_SPECIAL_PROFILE_EVENT::PRESENT) * 1000.0f));

					currPos = DrawTextLine(mpImGUIContext, currPos, 0.0f, vOffset.y);
				}

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