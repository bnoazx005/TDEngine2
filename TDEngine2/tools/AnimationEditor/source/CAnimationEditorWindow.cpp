#include "../include/CAnimationEditorWindow.h"


namespace TDEngine2
{
	CAnimationEditorWindow::CAnimationEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CAnimationEditorWindow::Init(IResourceManager* pResourceManager)
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

	void CAnimationEditorWindow::SetAnimationResourceHandle(TResourceId handle)
	{
		mCurrAnimationResourceHandle = handle;
		mpCurrAnimationClip = mpResourceManager->GetResource<IAnimationClip>(mCurrAnimationResourceHandle);
	}

	void CAnimationEditorWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(780.0f, 380.0f),
			TVector2(1000.0f, 550.0f),
		};

		if (mpImGUIContext->BeginWindow("Animation Editor", isEnabled, params))
		{
			_drawToolbar();
			_drawTimelineEditorGroup();
		}

		mpImGUIContext->EndWindow();

		mIsVisible = isEnabled;
	}

	void CAnimationEditorWindow::_drawToolbar()
	{
		if (mpImGUIContext->BeginChildWindow("Toolbar", TVector2(mpImGUIContext->GetWindowWidth(), 15.0f)))
		{
			mpImGUIContext->BeginHorizontal();

			static const TVector2 buttonSize(20.0f, 20.0f);

			mpImGUIContext->Button("*##Record", buttonSize);
			mpImGUIContext->Button("<|##BackStep", buttonSize);
			mpImGUIContext->Button(">##Play", buttonSize);
			mpImGUIContext->Button("|>##ForwardStep", buttonSize);

			{
				mpImGUIContext->Label("Duration: ");

				mpImGUIContext->SetItemWidth(50.0f, [this]
				{
					F32 duration = mpCurrAnimationClip ? mpCurrAnimationClip->GetDuration() : 0.0f;
					mpImGUIContext->FloatField("##Duration", duration, [&duration, this]
					{
						if (!mpCurrAnimationClip)
						{
							return;
						}

						mpCurrAnimationClip->SetDuration(duration);
					});
				});
			}

			{
				mpImGUIContext->Label("Dope Sheet");
				mpImGUIContext->Checkbox("##DopeSheetMode", mIsDopeSheetModeEnabled);
			}

			mpImGUIContext->EndHorizontal();

			mpImGUIContext->EndChildWindow();
		}
	}

	void CAnimationEditorWindow::_drawTimelineEditorGroup()
	{
		mpImGUIContext->SetCursorScreenPos(mpImGUIContext->GetCursorScreenPos() + TVector2(0.0f, 10.0f));
		mpImGUIContext->VerticalSeparator(mpImGUIContext->GetWindowWidth() * 0.3f,
			std::bind(&CAnimationEditorWindow::_drawTracksHierarchy, this, std::placeholders::_1), 
			std::bind(&CAnimationEditorWindow::_drawTimelineEditor, this, std::placeholders::_1));
	}

	void CAnimationEditorWindow::_drawTracksHierarchy(F32 blockWidth)
	{
		mpImGUIContext->BeginChildWindow("##TracksHierarchyWidget", TVector2(blockWidth - 10.0f, mpImGUIContext->GetWindowHeight() * 0.73f));
		{

		}
		mpImGUIContext->EndChildWindow();

		mpImGUIContext->BeginChildWindow("##TracksHierarchyToolbarWidget", TVector2(blockWidth - 10.0f, 35.0f));
		{
			if (mpImGUIContext->Button("Add Property", TVector2(mpImGUIContext->GetWindowWidth() * 0.5f, 25.0f)))
			{

			}
		}
		mpImGUIContext->EndChildWindow();
	}

	void CAnimationEditorWindow::_drawTimelineEditor(F32 blockWidth)
	{
		mpImGUIContext->BeginChildWindow("##TracksTimelineWidget", TVector2(blockWidth - 10.0f, mpImGUIContext->GetWindowHeight() * 0.8f));
		{
			F32 playbackTime = 0.0f;

			mpImGUIContext->SetItemWidth(mpImGUIContext->GetWindowWidth(), [&playbackTime, this]
			{
				mpImGUIContext->FloatSlider("##Cursor", playbackTime, 0.0f, mpCurrAnimationClip ? mpCurrAnimationClip->GetDuration() : 0.0f, [&playbackTime, this] {});
			});

			if (mIsDopeSheetModeEnabled)
			{
				_drawDopesheetWidget(playbackTime);
			}
			else
			{

			}
		}
		mpImGUIContext->EndChildWindow();
	}

	void CAnimationEditorWindow::_drawDopesheetWidget(F32 currPlaybackTime)
	{
		TVector2 cursorPos = mpImGUIContext->GetCursorScreenPos();

		const F32 duration = (mpCurrAnimationClip ? mpCurrAnimationClip->GetDuration() : 1.0f);
		const F32 timelineWidth = mpImGUIContext->GetWindowWidth();
		const F32 timelineHeight = mpImGUIContext->GetWindowHeight() * 0.95f;
		const F32 pixelsPerSecond = timelineWidth / duration;

		mpImGUIContext->DrawRect(TRectF32(cursorPos.x, cursorPos.y, timelineWidth, timelineHeight), TColor32F(0.2f, 0.2f, 0.2f, 1.0f), true, 1.0f);

		/// \note Draw tracks
		//mpImGUIContext->DrawLine()

		/// \note Draw vertical lines which determine seconds
		for (F32 t = 0.0f; CMathUtils::IsLessOrEqual(t, duration); t += 1.0f)
		{
			mpImGUIContext->DrawLine(cursorPos + TVector2(t * pixelsPerSecond, 0.0f), cursorPos + TVector2(t * pixelsPerSecond, timelineHeight), TColor32F(0.25f, 0.25f, 0.25f, 1.0f));
		}

		/// \note Draw a cursor
		mpImGUIContext->DrawLine(cursorPos + TVector2(currPlaybackTime * pixelsPerSecond, 0.0f), cursorPos + TVector2(currPlaybackTime * pixelsPerSecond, timelineHeight), TColorUtils::mWhite);
	}


	TDE2_API IEditorWindow* CreateAnimationEditorWindow(IResourceManager* pResourceManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CAnimationEditorWindow, result, pResourceManager);
	}
}