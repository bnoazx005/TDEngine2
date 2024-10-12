#include "../../include/editor/CStatsViewerWindow.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/editor/CStatsCounters.h"


#if TDE2_EDITORS_ENABLED

#define META_EXPORT_GRAPHICS_SECTION
#include "../../include/metadata.h"

namespace TDEngine2
{
	CStatsViewerWindow::CStatsViewerWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CStatsViewerWindow::Init(TPtr<IGraphicsContext> pGraphicsContext)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		mpGraphicsContext = pGraphicsContext;

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
			ZeroVector2,
			false, 
			true,
			true,
			true
		};

		if (mpImGUIContext->BeginWindow("Stats Viewer", isEnabled, params))
		{
			mpImGUIContext->SetCursorScreenPos(TVector2(10.0f/*mpImGUIContext->GetWindowWidth() * 0.77f*/, 10.0f));

			if (mpImGUIContext->BeginChildWindow("##Stats", TVector2(400.0f, 600.0f)))
			{
				TVector2 currPos = mpImGUIContext->GetCursorScreenPos();
				const TVector2 vOffset(0.0f, 15.0f);

				mpImGUIContext->DrawText(currPos, TColorUtils::mWhite, "Stats: ");

				currPos = DrawTextLine(mpImGUIContext, currPos + vOffset, 0.0f, vOffset.y, "FPS: ", std::to_string(static_cast<I32>(1.0f / CMathUtils::Max(1e-3f, mCurrDeltaTime))));
				currPos = DrawTextLine(mpImGUIContext, currPos + vOffset, 0.0f, vOffset.y, "Frame: ", std::to_string(TFrameCounter::mGlobalFrameNumber));
				
				{
					// Frame
					currPos = DrawTextLine(mpImGUIContext, currPos, 0.0f, vOffset.y);
					currPos = DrawTextLine(mpImGUIContext, currPos, 0.0f, vOffset.y, "Frame Time (ms): ", std::to_string(mCurrDeltaTime * 1000.0f));
					{
						currPos = DrawTextLine(mpImGUIContext, currPos, 20.0f, vOffset.y, "Update Time (ms): ",
							std::to_string(CPerfProfiler::Get()->GetAverageTimeByEventName(E_SPECIAL_PROFILE_EVENT::UPDATE) * 1000.0f));

						currPos = DrawTextLine(mpImGUIContext, currPos, 25.0f, vOffset.y, "World Update (ms): ",
							std::to_string(CPerfProfiler::Get()->GetAverageTimeByEventName(E_SPECIAL_PROFILE_EVENT::WORLD_UPDATE) * 1000.0f));

						currPos = DrawTextLine(mpImGUIContext, currPos, 25.0f, vOffset.y, "World Sync (ms): ",
							std::to_string(CPerfProfiler::Get()->GetAverageTimeByEventName(E_SPECIAL_PROFILE_EVENT::WORLD_SYSTEMS_SYNC) * 1000.0f));

						currPos = DrawTextLine(mpImGUIContext, currPos, 25.0f, vOffset.y, "Audio Update (ms): ",
							std::to_string(CPerfProfiler::Get()->GetAverageTimeByEventName(E_SPECIAL_PROFILE_EVENT::AUDIO_UPDATE) * 1000.0f));

						currPos = DrawTextLine(mpImGUIContext, currPos, 20.0f, vOffset.y, "Render Time (ms): ",
							std::to_string(CPerfProfiler::Get()->GetAverageTimeByEventName(E_SPECIAL_PROFILE_EVENT::RENDER) * 1000.0f));

						currPos = DrawTextLine(mpImGUIContext, currPos, 20.0f, vOffset.y, "Present (ms): ",
							std::to_string(CPerfProfiler::Get()->GetAverageTimeByEventName(E_SPECIAL_PROFILE_EVENT::PRESENT) * 1000.0f));

						currPos = currPos + TVector2(0.0f, vOffset.y);
					}
					
					// Render
					{
						currPos = DrawTextLine(mpImGUIContext, currPos, 0.0f, vOffset.y, "Draw Calls: ", std::to_string(CStatsCounters::mDrawCallsCount));
						currPos = currPos + TVector2(0.0f, vOffset.y);
					}

					// ECS
					{
						currPos = DrawTextLine(mpImGUIContext, currPos, 0.0f, vOffset.y, "Entities Count: ", std::to_string(CStatsCounters::mTotalEntitiesCount));
						currPos = DrawTextLine(mpImGUIContext, currPos, 0.0f, vOffset.y, "Components Count: ", std::to_string(CStatsCounters::mTotalComponentsCount));
						currPos = currPos + TVector2(0.0f, vOffset.y);
					}

					// Resources
					{
						currPos = DrawTextLine(mpImGUIContext, currPos, 0.0f, vOffset.y, "Loaded Resources: ", std::to_string(CStatsCounters::mLoadedResourcesCount));
						currPos = currPos + TVector2(0.0f, vOffset.y);
					}

					// System info
					{
						currPos = DrawTextLine(mpImGUIContext, currPos, 0.0f, vOffset.y, "Graphics Info:", " ");

						const TGraphicsContextInfo& graphicsInfo  = mpGraphicsContext->GetContextInfo();
						const TVideoAdapterInfo& videoAdapterInfo = mpGraphicsContext->GetInfo();

						currPos = DrawTextLine(mpImGUIContext, currPos, 20.0f, vOffset.y, "GAPI:", Meta::EnumTrait<E_GRAPHICS_CONTEXT_GAPI_TYPE>::ToString(graphicsInfo.mGapiType));
						currPos = DrawTextLine(mpImGUIContext, currPos, 20.0f, vOffset.y, "Video Memory:", Wrench::StringUtils::Format("{0} / {1} MiB used", videoAdapterInfo.mUsedVideoMemory / (1 << 20), videoAdapterInfo.mAvailableVideoMemory / (1 << 20)));

						currPos = currPos + TVector2(0.0f, vOffset.y);
					}
				}

				mpImGUIContext->EndChildWindow();
			}
		}

		mpImGUIContext->EndWindow();

		mIsVisible = isEnabled;
	}


	TDE2_API IEditorWindow* CreateStatsViewerWindow(TPtr<IGraphicsContext> pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CStatsViewerWindow, result, pGraphicsContext);
	}
}

#endif