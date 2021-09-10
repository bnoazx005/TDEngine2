#include "../../include/editor/CAnimationCurveEditorWindow.h"
#include "../../include/graphics/animation/CAnimationCurve.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/math/MathUtils.h"


namespace TDEngine2
{
	static TVector2 ApplyCurveToScreenTransform(const CAnimationCurveEditorWindow::TCurveTransformParams& params, const TVector2& p)
	{
		auto&& curveBounds = params.mCurveBounds;
		auto&& cursorPos = params.mCursorPosition;
		const F32& width = params.mFrameWidth;
		const F32& height = params.mFrameHeight;

		TDE2_ASSERT(curveBounds.width > 0.0f && curveBounds.height > 0.0f);

		return
		{
			cursorPos.x + (p.x - curveBounds.x) / std::max<F32>(1e-3f, curveBounds.width) * width,
			cursorPos.y + (1.0f - (p.y - curveBounds.y) / std::max<F32>(1e-3f, curveBounds.height)) * height
		};
	};


	static TVector2 ApplyScreenToCurveTransform(const CAnimationCurveEditorWindow::TCurveTransformParams& params, const TVector2& p)
	{
		auto&& curveBounds = params.mCurveBounds;
		auto&& cursorPos = params.mCursorPosition;
		const F32& width = params.mFrameWidth;
		const F32& height = params.mFrameHeight;

		TDE2_ASSERT(curveBounds.width > 0.0f && curveBounds.height > 0.0f && width > 0.0f && height > 0.0f);

		return
		{
			(p.x - cursorPos.x) / width * curveBounds.width + curveBounds.x,
			(1.0f - (p.y - cursorPos.y) / height) * curveBounds.height + curveBounds.y
		};
	};


	static void DrawCurvePoint(IImGUIContext* pImGUIContext, CAnimationCurve* pCurve, I32 id, const TVector2& pos, const CAnimationCurveEditorWindow::TCurveTransformParams& invTransformParams,
							   F32 handlePointSize, F32 handlePointButtonSize, const TAnimationCurveEditorParams::TActionCallback& onCurveClicked, bool shouldIgnoreInput)
	{
		pImGUIContext->DisplayIDGroup(id, [shouldIgnoreInput, pImGUIContext, pCurve, handlePointSize, handlePointButtonSize, onCurveClicked, p0 = pos, &invTransformParams, id]
		{
			pImGUIContext->DrawCircle(p0, handlePointSize, true, TColorUtils::mGreen);

			auto pos = pImGUIContext->GetCursorScreenPos();

			pImGUIContext->SetCursorScreenPos(p0 - TVector2(handlePointButtonSize * 0.5f));
			
			pImGUIContext->Button(Wrench::StringUtils::GetEmptyStr(), TVector2(handlePointButtonSize), {}, true);

			if (!shouldIgnoreInput)
			{
				if (pImGUIContext->IsItemActive() && pImGUIContext->IsMouseDragging(0))
				{
					auto&& mousePos = pImGUIContext->GetMousePosition();

					if (auto pPoint = pCurve->GetPoint(static_cast<U32>(id)))
					{
						const TVector2& curvePointDelta = ApplyScreenToCurveTransform(invTransformParams, mousePos);

						pPoint->mTime = curvePointDelta.x;
						pPoint->mValue = curvePointDelta.y;
					}
				}

				if (!pImGUIContext->IsMouseDragging(0))
				{
					pCurve->UpdateBounds();
				}
			}

			if (onCurveClicked && pImGUIContext->IsItemHovered() && pImGUIContext->IsMouseClicked(0))
			{
				onCurveClicked();
			}

			pImGUIContext->SetCursorScreenPos(pos);
		});
	}

	TVector2 DrawControlPoint(IImGUIContext* pImGUIContext, I32 id, const TVector2& pos, const TVector2& controlPointPos, const CAnimationCurveEditorWindow::TCurveTransformParams& invTransformParams,
							  F32 handlePointSize, F32 handlePointButtonSize, bool shouldIgnoreInput)
	{
		TVector2 changedControlPoint = controlPointPos;

		changedControlPoint.x -= pos.x;
		changedControlPoint.y -= pos.y;

		pImGUIContext->DisplayIDGroup(id, [pImGUIContext, handlePointSize, handlePointButtonSize, shouldIgnoreInput, &changedControlPoint, &p = pos, &invTransformParams, &cp = controlPointPos]
		{
			pImGUIContext->DrawLine(p, cp, TColorUtils::mWhite);
			pImGUIContext->DrawCircle(cp, handlePointSize, false, TColorUtils::mWhite);

			auto pos = pImGUIContext->GetCursorScreenPos();

			pImGUIContext->SetCursorScreenPos(cp - TVector2(0.5f * handlePointButtonSize));

			if (!shouldIgnoreInput)
			{
				pImGUIContext->Button("", TVector2(handlePointButtonSize), {}, true);

				if (pImGUIContext->IsItemActive() && pImGUIContext->IsMouseDragging(0))
				{
					auto&& mousePos = pImGUIContext->GetMousePosition();

					changedControlPoint.x = mousePos.x - p.x;
					changedControlPoint.y = mousePos.y - p.y;
				}
			}

			pImGUIContext->SetCursorScreenPos(pos);
		});

		return changedControlPoint;
	}

	static void DrawCurveLine(IImGUIContext* pImGUIContext, CAnimationCurve* pCurve, F32 width, F32 height, const TVector2& cursorPos, I32 controlPointsOffset,
							  const TColor32F& curveColor, F32 handlePointSize, F32 handlePointButtonSize, const TAnimationCurveEditorParams::TActionCallback& onCurveClicked, bool shouldIgnoreInput,
							  bool useCustomBounds = false, const TRectF32* pCustomGridBounds = nullptr)
	{
		if (!pCurve)
		{
			return;
		}

		const TRectF32& curveBounds = useCustomBounds ? *pCustomGridBounds : pCurve->GetBounds();

		const CAnimationCurveEditorWindow::TCurveTransformParams transformParams{ curveBounds, cursorPos, width, height };

		for (auto it = pCurve->begin(); it != pCurve->end(); ++it)
		{
			auto&& currPoint = *it;

			const TVector2& initCurrPos{ currPoint.mTime, currPoint.mValue };

			auto p0 = ApplyCurveToScreenTransform(transformParams, initCurrPos);
			TVector2 p1, t0, t1;

			if (it != pCurve->end() - 1)
			{
				auto&& nextPoint = *(it + 1);
				const TVector2& initNextPos{ nextPoint.mTime, nextPoint.mValue };

				p1 = ApplyCurveToScreenTransform(transformParams, initNextPos);
				t0 = ApplyCurveToScreenTransform(transformParams, initCurrPos + currPoint.mOutTangent);
				t1 = ApplyCurveToScreenTransform(transformParams, initNextPos + nextPoint.mInTangent);

				pImGUIContext->DrawCubicBezier(p0, t0, p1, t1, curveColor);
			}

			DrawCurvePoint(pImGUIContext, pCurve, static_cast<I32>(std::distance(pCurve->begin(), it)), p0, transformParams, handlePointSize, handlePointButtonSize, onCurveClicked, shouldIgnoreInput);

			if (it < pCurve->end() - 1)
			{
				t0 = ApplyCurveToScreenTransform(transformParams, initCurrPos + currPoint.mOutTangent);

				t0 = DrawControlPoint(pImGUIContext, controlPointsOffset + static_cast<I32>(std::distance(pCurve->begin(), it)), p0, t0, transformParams, handlePointSize, handlePointButtonSize, shouldIgnoreInput);
				t0 = ApplyScreenToCurveTransform(transformParams, p0 + t0) - initCurrPos;

				currPoint.mOutTangent = t0;
			}

			if (it > pCurve->begin())
			{
				t1 = ApplyCurveToScreenTransform(transformParams, initCurrPos + currPoint.mInTangent);

				t1 = DrawControlPoint(pImGUIContext, 2 * controlPointsOffset + static_cast<I32>(std::distance(pCurve->begin(), it)), p0, t1, transformParams, handlePointSize, handlePointButtonSize, shouldIgnoreInput);
				t1 = ApplyScreenToCurveTransform(transformParams, p0 + t1) - initCurrPos;

				currPoint.mInTangent = t1;
			}
		}
	}


	static void HandleCurveCursor(IImGUIContext* pImGUIContext, CAnimationCurve* pCurve, F32 width, F32 height, const TVector2& cursorPos, bool shouldIgnoreInput)
	{
		if (!pCurve)
		{
			return;
		}

		const TRectF32& curveBounds = pCurve->GetBounds();

		const CAnimationCurveEditorWindow::TCurveTransformParams transformParams{ curveBounds, cursorPos, width, height };

		const TVector2& mousePos = pImGUIContext->GetMousePosition();
		const TVector2 curveMousePos = ApplyScreenToCurveTransform(transformParams, mousePos);

		const F32 curveValue = pCurve->Sample(curveMousePos.x);

		if (CMathUtils::Abs(curveValue - curveMousePos.y) < 0.2f)
		{
			// draw a cursor if a user moves it right near a curve
			pImGUIContext->DrawCircle(ApplyCurveToScreenTransform(transformParams, TVector2(curveMousePos.x, curveValue)), 4.0f, false, TColorUtils::mYellow);

			if (!shouldIgnoreInput)
			{
				if (pImGUIContext->IsMouseDoubleClicked(0))
				{
					static const TVector2 defaultControlPoint{ 0.25f * RightVector2 };

					pCurve->AddPoint({ curveMousePos.x, /*curveValue*/ 0.5f, -defaultControlPoint, defaultControlPoint });
				}
			}
		}
	}


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


	static const TRectF32 DefaultFrameBounds{ 0.0f, 0.0f, 1.0f, 1.0f };


	E_RESULT_CODE CAnimationCurveEditorWindow::DrawCurveEditor(IImGUIContext* pImGUIContext, const TAnimationCurveEditorParams& params, CAnimationCurve* pCurve)
	{
		if (!pImGUIContext || !pCurve)
		{
			return RC_INVALID_ARGS;
		}

		const F32 width = params.mFrameWidth;
		const F32 height = params.mFrameHeight;
		
		IImGUIContext::TPlotGridParams gridParams;
		{
			gridParams.mWidth               = width;
			gridParams.mHeight              = height;
			gridParams.mFrame               = pCurve ? (params.mUseCustomGridBounds ? params.mGridBounds : pCurve->GetBounds()) : DefaultFrameBounds;
			gridParams.mIsGridEnabled       = params.mIsGridVisible;
			gridParams.mIsBackgroundEnabled = params.mIsBackgroundVisible;
			gridParams.mCurveColor          = params.mCurveColor;
		}

		pImGUIContext->DrawPlotGrid("Plot", gridParams, [=, curveColor = gridParams.mCurveColor, shouldIgnoreInput = params.mShouldIgnoreInput](auto&& pos)
		{
			DrawCurveLine(pImGUIContext, pCurve, width, height, pos, mControlPointsOffset, curveColor, mHandlePointSize, mHandlePointButtonSize, params.mOnCurveClickedCallback, shouldIgnoreInput,
						  params.mUseCustomGridBounds, &params.mGridBounds);
			HandleCurveCursor(pImGUIContext, pCurve, width, height, pos, shouldIgnoreInput);
		});

		return RC_OK;
	}


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
			DrawCurveEditor(mpImGUIContext, { mpImGUIContext->GetWindowWidth() - 15.0f, mpImGUIContext->GetWindowHeight() - 100.0f }, mpCurrTargetCurve);

			mpImGUIContext->Button("Cancel", { 100.0f, 25.0f }, [this] 
			{ 
				mpCurrTargetCurve = nullptr; 
			});
		}

		mpImGUIContext->EndWindow();

		mIsVisible = isEnabled;
	}


	TDE2_API IEditorWindow* CreateAnimationCurveEditorWindow(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CAnimationCurveEditorWindow, result);
	}
}