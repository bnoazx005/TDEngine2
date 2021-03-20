#include "../../include/editor/CAnimationCurveEditorWindow.h"
#include "../../include/graphics/animation/CAnimationCurve.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/math/MathUtils.h"


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
			
			mpImGUIContext->DrawPlotGrid("Plot", { width, height, 5, 5, mpCurrTargetCurve ? mpCurrTargetCurve->GetBounds() : DefaultFrameBounds }, [&](auto&& pos)
			{
				_drawCurveLine(width, height, pos);
				_handleCurveCursor(width, height, pos);
			});

			mpImGUIContext->Button("Cancel", { 100.0f, 25.0f }, [this] 
			{ 
				mpCurrTargetCurve = nullptr; 
			});
		}

		mpImGUIContext->EndWindow();

		mIsVisible = isEnabled;
	}


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
		auto&& cursorPos   = params.mCursorPosition;
		const F32& width   = params.mFrameWidth;
		const F32& height  = params.mFrameHeight;

		TDE2_ASSERT(curveBounds.width > 0.0f && curveBounds.height > 0.0f && width > 0.0f && height > 0.0f);

		return
		{
			(p.x - cursorPos.x) / width * curveBounds.width + curveBounds.x,
			(1.0f - (p.y - cursorPos.y) / height) * curveBounds.height + curveBounds.y
		};
	};


	void CAnimationCurveEditorWindow::_drawCurveLine(F32 width, F32 height, const TVector2& cursorPos)
	{
		if (!mpCurrTargetCurve)
		{
			return;
		}

		const TRectF32& curveBounds = mpCurrTargetCurve->GetBounds();

		const TCurveTransformParams transformParams { curveBounds, cursorPos, width, height };

		for (auto it = mpCurrTargetCurve->begin(); it != mpCurrTargetCurve->end(); ++it)
		{
			auto&& currPoint = *it;

			const TVector2& initCurrPos { currPoint.mTime, currPoint.mValue };

			auto p0 = ApplyCurveToScreenTransform(transformParams, initCurrPos);
			TVector2 p1, t0, t1;

			if (it != mpCurrTargetCurve->end() - 1)
			{
				auto&& nextPoint = *(it + 1);
				const TVector2& initNextPos{ nextPoint.mTime, nextPoint.mValue };

				p1 = ApplyCurveToScreenTransform(transformParams, initNextPos);
				t0 = ApplyCurveToScreenTransform(transformParams, initCurrPos + currPoint.mOutTangent);
				t1 = ApplyCurveToScreenTransform(transformParams, initNextPos + nextPoint.mInTangent);

				mpImGUIContext->DrawCubicBezier(p0, t0, p1, t1, TColorUtils::mGreen);
			}

			_drawCurvePoint(static_cast<I32>(std::distance(mpCurrTargetCurve->begin(), it)), p0, transformParams);

			if (it < mpCurrTargetCurve->end() - 1)
			{
				t0 = ApplyCurveToScreenTransform(transformParams, initCurrPos + currPoint.mOutTangent);

				t0 = _drawControlPoint(mControlPointsOffset + static_cast<I32>(std::distance(mpCurrTargetCurve->begin(), it)), p0, t0, transformParams);
				t0 = ApplyScreenToCurveTransform(transformParams, p0 + t0) - initCurrPos;

				currPoint.mOutTangent = t0;
			}

			if (it > mpCurrTargetCurve->begin())
			{
				t1 = ApplyCurveToScreenTransform(transformParams, initCurrPos + currPoint.mInTangent);

				t1 = _drawControlPoint(2 * mControlPointsOffset + static_cast<I32>(std::distance(mpCurrTargetCurve->begin(), it)), p0, t1, transformParams);
				t1 = ApplyScreenToCurveTransform(transformParams, p0 + t1) - initCurrPos;

				currPoint.mInTangent = t1;
			}
		}
	}

	void CAnimationCurveEditorWindow::_drawCurvePoint(I32 id, const TVector2& pos, const TCurveTransformParams& invTransformParams)
	{
		mpImGUIContext->DisplayIDGroup(id, [this, &p0 = pos, &invTransformParams, id]
		{
			mpImGUIContext->DrawCircle(p0, mHandlePointSize, true, TColorUtils::mGreen);

			auto pos = mpImGUIContext->GetCursorScreenPos();

			mpImGUIContext->SetCursorScreenPos(p0 - TVector2(mHandlePointButtonSize * 0.5f));
			{
				mpImGUIContext->Button(Wrench::StringUtils::GetEmptyStr(), TVector2(mHandlePointButtonSize), {}, true);

				if (mpImGUIContext->IsItemActive() && mpImGUIContext->IsMouseDragging(0))
				{
					auto&& mousePos = mpImGUIContext->GetMousePosition();
					const TVector2 mouseDelta = mpImGUIContext->GetMouseDragDelta(0);

					if (auto pPoint = mpCurrTargetCurve->GetPoint(static_cast<U32>(id)))
					{
						const TVector2& curvePointDelta = ApplyScreenToCurveTransform(invTransformParams, p0 + mouseDelta);

						pPoint->mTime = curvePointDelta.x;
						pPoint->mValue = curvePointDelta.y;
					}
				}
			}
			mpImGUIContext->SetCursorScreenPos(pos);
		});
	}

	TVector2 CAnimationCurveEditorWindow::_drawControlPoint(I32 id, const TVector2& pos, const TVector2& controlPointPos, const TCurveTransformParams& invTransformParams)
	{
		TVector2 changedControlPoint = controlPointPos;

		changedControlPoint.x -= pos.x;
		changedControlPoint.y -= pos.y;

		mpImGUIContext->DisplayIDGroup(id, [this, &changedControlPoint, &p = pos, &invTransformParams, &cp = controlPointPos]
		{
			mpImGUIContext->DrawLine(p, cp, TColorUtils::mWhite);
			mpImGUIContext->DrawCircle(cp, mHandlePointSize, false, TColorUtils::mWhite);

			auto pos = mpImGUIContext->GetCursorScreenPos();

			mpImGUIContext->SetCursorScreenPos(cp - TVector2(0.5f * mHandlePointButtonSize));

			mpImGUIContext->Button("", TVector2(mHandlePointButtonSize), {}, true);

			if (mpImGUIContext->IsItemActive() && mpImGUIContext->IsMouseDragging(0))
			{
				auto&& mousePos = mpImGUIContext->GetMousePosition();

				changedControlPoint.x = mousePos.x - p.x;
				changedControlPoint.y = mousePos.y - p.y;
			}

			mpImGUIContext->SetCursorScreenPos(pos);
		});

		return changedControlPoint;
	}

	void CAnimationCurveEditorWindow::_handleCurveCursor(F32 width, F32 height, const TVector2& cursorPos)
	{
		if (!mpCurrTargetCurve)
		{
			return;
		}

		const TRectF32& curveBounds = mpCurrTargetCurve->GetBounds();

		const TCurveTransformParams transformParams{ curveBounds, cursorPos, width, height };

		const TVector2& mousePos = mpImGUIContext->GetMousePosition();

		const F32 xCoord = ApplyScreenToCurveTransform(transformParams, mousePos).x;

		const F32 curveValue = mpCurrTargetCurve->Sample(xCoord);

		if (CMathUtils::Abs(curveValue - 1.f) < 1e-3f)
		{
			// draw a cursor if a user moves it right near a curve
			mpImGUIContext->DrawCircle(ApplyCurveToScreenTransform(transformParams, TVector2(xCoord, curveValue)), 4.0f, false, TColorUtils::mYellow);

			if (mpImGUIContext->IsMouseDoubleClicked(0))
			{
				static const TVector2 defaultControlPoint{ 0.25f * RightVector2 };

				mpCurrTargetCurve->AddPoint({ xCoord, /*curveValue*/ 0.5f, -defaultControlPoint, defaultControlPoint });
			}
		}
	}


	TDE2_API IEditorWindow* CreateAnimationCurveEditorWindow(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CAnimationCurveEditorWindow, result);
	}
}