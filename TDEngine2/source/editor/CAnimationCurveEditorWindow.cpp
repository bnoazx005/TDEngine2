#include "../../include/editor/CAnimationCurveEditorWindow.h"
#include "../../include/graphics/animation/CAnimationCurve.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/math/TVector2.h"


namespace TDEngine2
{
	CAnimationCurveEditorWindow::CAnimationCurveEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CAnimationCurveEditorWindow::Init()
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		mpCurrTargetCurve = nullptr;

		mIsInitialized = true;
		mIsVisible = true;

		return RC_OK;
	}

	E_RESULT_CODE CAnimationCurveEditorWindow::Free()
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

	E_RESULT_CODE CAnimationCurveEditorWindow::SetCurveForEditing(CAnimationCurve* pCurve)
	{
		if (!pCurve)
		{
			return RC_INVALID_ARGS;
		}

		mpCurrTargetCurve = pCurve;

		return RC_OK;
	}


	static const TRectF32 DefaultFrameBounds { 0.0f, 0.0f, 1.0f, 1.0f };


	void CAnimationCurveEditorWindow::_onDraw()
	{
		bool isEnabled = mIsVisible && mpCurrTargetCurve;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(450.0f, 200.0f),
			TVector2(1000.0f, 550.0f),
		};

		if (mpImGUIContext->BeginWindow("Curve Editor", isEnabled, params))
		{
			const F32 width  = mpImGUIContext->GetWindowWidth() - 15.0f;
			const F32 height = mpImGUIContext->GetWindowHeight() - 100.0f;

#if 1 // OLD_IMPLEMENTATION

			static TVector2 tangents[2]
			{
				TVector2(200.0f, 0.0f),
				TVector2(-200.0f, 0.0f)
			};

			TVector2* pT0 = &tangents[0];
			TVector2* pT1 = &tangents[1];

			mpImGUIContext->DrawPlotGrid("Plot", { width, height, 5, 5, mpCurrTargetCurve ? mpCurrTargetCurve->GetBounds() : DefaultFrameBounds }, [&](auto&& pos)
			{
				auto a = pos;
				auto b = pos + TVector2(width, height);

				_drawCurveLine(width, height, pos);

				/*mpImGUIContext->DrawCubicBezier(a, a + tangents[0], b, b + tangents[1], TColorUtils::mGreen);

				mpImGUIContext->DisplayIDGroup(0, [&, t = *pT0]
					{
						mpImGUIContext->DrawLine(a, a + t, TColorUtils::mWhite);
						mpImGUIContext->DrawCircle(a + t, 5.0f, true, TColorUtils::mBlue);

						auto pos = mpImGUIContext->GetCursorScreenPos();

						mpImGUIContext->SetCursorScreenPos(a + t);

						mpImGUIContext->Button("", TVector2(30.0f, 30.0f), {}, true);

						if (mpImGUIContext->IsItemActive() && mpImGUIContext->IsMouseDragging(0))
						{
							auto&& mousePos = mpImGUIContext->GetMousePosition();

							pT0->x = mousePos.x - a.x;
							pT0->y = mousePos.y - a.y;
						}

						mpImGUIContext->SetCursorScreenPos(pos);
					});

					mpImGUIContext->DisplayIDGroup(1, [&, t = tangents[1]]
					{
						mpImGUIContext->DrawLine(b, b + t, TColorUtils::mWhite);
						mpImGUIContext->DrawCircle(b + t, 5.0f, true, TColorUtils::mBlue);

						auto pos = mpImGUIContext->GetCursorScreenPos();

						mpImGUIContext->SetCursorScreenPos(b + t);

						mpImGUIContext->Button("", TVector2(30.0f, 30.0f), {}, true);

						if (mpImGUIContext->IsItemActive() && mpImGUIContext->IsMouseDragging(0))
						{
							auto&& mousePos = mpImGUIContext->GetMousePosition();

							pT1->x = mousePos.x - b.x;
							pT1->y = mousePos.y - b.y;
						}


						mpImGUIContext->SetCursorScreenPos(pos);
					});*/
			});
#endif

			mpImGUIContext->Button("Cancel", { 100.0f, 25.0f }, [this] 
			{ 
				mpCurrTargetCurve = nullptr; 
			});
		}

		mpImGUIContext->EndWindow();

		mIsVisible = isEnabled;
	}

	void CAnimationCurveEditorWindow::_drawCurveLine(F32 width, F32 height, const TVector2& cursorPos)
	{
		if (!mpCurrTargetCurve)
		{
			return;
		}

		const TRectF32& curveBounds = mpCurrTargetCurve->GetBounds();

		auto transformFromCurveToScreen = [&curveBounds, &cursorPos, width, height](const TVector2& p) -> TVector2
		{
			TDE2_ASSERT(curveBounds.width > 0.0f && curveBounds.height > 0.0f);

			return
			{
				cursorPos.x + (p.x - curveBounds.x) / std::max<F32>(1e-3f, curveBounds.width) * width,
				cursorPos.y + (1.0f - (p.y - curveBounds.y) / std::max<F32>(1e-3f, curveBounds.height)) * height
			};
		};

		for (auto it = mpCurrTargetCurve->begin(); it != mpCurrTargetCurve->end() - 1; ++it)
		{
			auto&& currPoint = *it;
			auto&& nextPoint = *(it + 1);

			const TVector2& initCurrPos { currPoint.mTime, currPoint.mValue };
			const TVector2& initNextPos { nextPoint.mTime, nextPoint.mValue };

			auto p0 = transformFromCurveToScreen(initCurrPos);
			auto p1 = transformFromCurveToScreen(initNextPos);
			auto t0 = transformFromCurveToScreen(initCurrPos + currPoint.mOutTangent);
			auto t1 = transformFromCurveToScreen(initNextPos + nextPoint.mInTangent);
			
			mpImGUIContext->DrawCubicBezier(p0, t0, p1, t1, TColorUtils::mGreen);
		}
	}


	TDE2_API IEditorWindow* CreateAnimationCurveEditorWindow(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CAnimationCurveEditorWindow, result);
	}
}