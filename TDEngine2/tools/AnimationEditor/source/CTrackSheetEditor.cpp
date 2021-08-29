#include "../include/CTrackSheetEditor.h"


namespace TDEngine2
{
	CTrackSheetEditor::CTrackSheetEditor() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CTrackSheetEditor::Init()
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CTrackSheetEditor::Free()
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

	E_RESULT_CODE CTrackSheetEditor::Draw(const TVector2& frameSizes)
	{
		if (mOnDrawImpl)
		{
			mOnDrawImpl(frameSizes);
			mIsEditing = true;
		}

		return RC_OK;
	}

	E_RESULT_CODE CTrackSheetEditor::Reset()
	{
		mIsEditing = false;
		return _resetState();
	}

	void CTrackSheetEditor::SetImGUIContext(IImGUIContext* pImGUIContext)
	{
		mpImGUIContext = pImGUIContext;
	}


	template <typename T>
	static CTrackSheetEditor::TCurveBindingCallback GenerateInitCurveCallback(T* pTrack, U32 memberOffset)
	{
		return [pTrack, memberOffset](const CScopedPtr<CAnimationCurve>& pCurve)
		{
			for (auto&& currKey : pTrack->GetKeys())
			{
				pCurve->AddPoint(CAnimationCurve::TKeyFrame{ currKey.mTime, *reinterpret_cast<const F32*>(&currKey.mValue + memberOffset) });
			}
		};
	}


	template <typename T>
	static CTrackSheetEditor::TCurveBindingCallback GenerateSerializePointsCallback(T* pTrack, U32 memberOffset)
	{
		return [pTrack, memberOffset](const CScopedPtr<CAnimationCurve>& pCurve)
		{
			pTrack->RemoveAllKeys();

			for (U32 i = 0; i < pCurve->GetPointsCount(); ++i)
			{
				if (auto&& pCurrPoint = pCurve->GetPoint(i))
				{
					if (auto pKeySample = pTrack->GetKey(pTrack->CreateKey(pCurrPoint->mTime)))
					{
						if (auto pKeyValue = reinterpret_cast<F32*>(&pKeySample->mValue + memberOffset))
						{
							*pKeyValue = pCurrPoint->mValue;
						}
					}					
				}
			}
		};
	}


	static CTrackSheetEditor::TCurveDrawCallback GenerateDrawCallback(IImGUIContext* pImGUIContext, CTrackSheetEditor::TCurveBindingsTable& curvesTable)
	{
		return [pImGUIContext, &curvesTable](const TVector2& frameSizes)
		{
			for (auto&& currCurveBindingInfo : curvesTable)
			{
				auto& curveInfo = currCurveBindingInfo.second;

				CAnimationCurveEditorWindow::DrawCurveEditor(pImGUIContext, { frameSizes.x, frameSizes.y }, curveInfo.mpCurve.Get());
			}
		};
	}


	E_RESULT_CODE CTrackSheetEditor::VisitVector2Track(CVector2AnimationTrack* pTrack)
	{
		E_RESULT_CODE result = _resetState();
		if (RC_OK != result)
		{
			return result;
		}

		const TRectF32 trackBounds{ 0.0f, 0.0f, pTrack->GetOwner()->GetDuration(), 1.0f };

		/// \note Create new curves 
		mCurvesTable.emplace("x", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, offsetof(TVector2, x)),
			GenerateSerializePointsCallback(pTrack, offsetof(TVector2, x)) });

		mCurvesTable.emplace("y", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, offsetof(TVector2, y)),
			GenerateSerializePointsCallback(pTrack, offsetof(TVector2, y)) });

		mOnDrawImpl = GenerateDrawCallback(mpImGUIContext, mCurvesTable);

		_initCurvesState();

		return RC_OK;
	}

	E_RESULT_CODE CTrackSheetEditor::VisitVector3Track(CVector3AnimationTrack* pTrack)
	{
		E_RESULT_CODE result = _resetState();
		if (RC_OK != result)
		{
			return result;
		}

		const TRectF32 trackBounds { 0.0f, 0.0f, pTrack->GetOwner()->GetDuration(), 1.0f };

		/// \note Create new curves 
		mCurvesTable.emplace("x", TTrack2CurveBindingInfo { CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) }, 
			GenerateInitCurveCallback(pTrack, offsetof(TVector3, x)),
			GenerateSerializePointsCallback(pTrack, offsetof(TVector3, x)) });

		mCurvesTable.emplace("y", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, offsetof(TVector3, y)),
			GenerateSerializePointsCallback(pTrack, offsetof(TVector3, y)) });

		mCurvesTable.emplace("z", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, offsetof(TVector3, z)),
			GenerateSerializePointsCallback(pTrack, offsetof(TVector3, z)) });

		mOnDrawImpl = GenerateDrawCallback(mpImGUIContext, mCurvesTable);

		_initCurvesState();

		return RC_OK;
	}

	E_RESULT_CODE CTrackSheetEditor::VisitQuaternionTrack(CQuaternionAnimationTrack* pTrack)
	{
		E_RESULT_CODE result = _resetState();
		if (RC_OK != result)
		{
			return result;
		}

		const TRectF32 trackBounds{ 0.0f, 0.0f, pTrack->GetOwner()->GetDuration(), 1.0f };

		/// \note Create new curves 
		mCurvesTable.emplace("x", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, offsetof(TQuaternion, x)),
			GenerateSerializePointsCallback(pTrack, offsetof(TQuaternion, x)) });

		mCurvesTable.emplace("y", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, offsetof(TQuaternion, y)),
			GenerateSerializePointsCallback(pTrack, offsetof(TQuaternion, y)) });

		mCurvesTable.emplace("z", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, offsetof(TQuaternion, z)),
			GenerateSerializePointsCallback(pTrack, offsetof(TQuaternion, z)) });

		mCurvesTable.emplace("w", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, offsetof(TQuaternion, w)),
			GenerateSerializePointsCallback(pTrack, offsetof(TQuaternion, w)) });

		mOnDrawImpl = GenerateDrawCallback(mpImGUIContext, mCurvesTable);

		_initCurvesState();

		return RC_OK;
	}

	E_RESULT_CODE CTrackSheetEditor::VisitColorTrack(CColorAnimationTrack* pTrack)
	{
		E_RESULT_CODE result = _resetState();
		if (RC_OK != result)
		{
			return result;
		}

		const TRectF32 trackBounds{ 0.0f, 0.0f, pTrack->GetOwner()->GetDuration(), 1.0f };

		/// \note Create new curves 
		mCurvesTable.emplace("r", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, offsetof(TColor32F, r)),
			GenerateSerializePointsCallback(pTrack, offsetof(TColor32F, r)) });

		mCurvesTable.emplace("g", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, offsetof(TColor32F, g)),
			GenerateSerializePointsCallback(pTrack, offsetof(TColor32F, g)) });

		mCurvesTable.emplace("b", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, offsetof(TColor32F, b)),
			GenerateSerializePointsCallback(pTrack, offsetof(TColor32F, b)) });

		mCurvesTable.emplace("a", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, offsetof(TColor32F, a)),
			GenerateSerializePointsCallback(pTrack, offsetof(TColor32F, a)) });

		mOnDrawImpl = GenerateDrawCallback(mpImGUIContext, mCurvesTable);

		_initCurvesState();

		return RC_OK;
	}

	E_RESULT_CODE CTrackSheetEditor::VisitFloatTrack(CFloatAnimationTrack* pTrack)
	{
		E_RESULT_CODE result = _resetState();
		if (RC_OK != result)
		{
			return result;
		}

		const TRectF32 trackBounds{ 0.0f, 0.0f, pTrack->GetOwner()->GetDuration(), 1.0f };

		/// \note Create new curves 
		mCurvesTable.emplace("value", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, 0),
			[pTrack](const CScopedPtr<CAnimationCurve>& pCurve)
			{
			} });

		_initCurvesState();

		return RC_OK;
	}

	E_RESULT_CODE CTrackSheetEditor::VisitIntegerTrack(CIntegerAnimationTrack* pTrack)
	{
		E_RESULT_CODE result = _resetState();
		if (RC_OK != result)
		{
			return result;
		}

		const TRectF32 trackBounds{ 0.0f, 0.0f, pTrack->GetOwner()->GetDuration(), 1.0f };

		/// \note Create new curves 
		mCurvesTable.emplace("value", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			[pTrack](const CScopedPtr<CAnimationCurve>& pCurve)
			{
			},
			[pTrack](const CScopedPtr<CAnimationCurve>& pCurve)
			{
			} });

		_initCurvesState();

		return RC_OK;
	}

	E_RESULT_CODE CTrackSheetEditor::VisitBooleanTrack(CBooleanAnimationTrack* pTrack)
	{
		E_RESULT_CODE result = _resetState();
		if (RC_OK != result)
		{
			return result;
		}

		const TRectF32 trackBounds{ 0.0f, 0.0f, pTrack->GetOwner()->GetDuration(), 1.0f };

		/// \note Create new curves 
		mCurvesTable.emplace("value", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			[pTrack](const CScopedPtr<CAnimationCurve>& pCurve)
			{
			},
			[pTrack](const CScopedPtr<CAnimationCurve>& pCurve)
			{
			} });

		_initCurvesState();

		return RC_OK;
	}

	E_RESULT_CODE CTrackSheetEditor::VisitEventTrack(CEventAnimationTrack* pTrack)
	{
		E_RESULT_CODE result = _resetState();
		if (RC_OK != result)
		{
			return result;
		}

		const TRectF32 trackBounds{ 0.0f, 0.0f, pTrack->GetOwner()->GetDuration(), 1.0f };

		/// \note Create new curves 
		mCurvesTable.emplace("value", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			[pTrack](const CScopedPtr<CAnimationCurve>& pCurve)
			{
			},
			[pTrack](const CScopedPtr<CAnimationCurve>& pCurve)
			{
			} });

		_initCurvesState();

		return RC_OK;
	}

	bool CTrackSheetEditor::IsEditing() const
	{
		return mIsEditing;
	}

	E_RESULT_CODE CTrackSheetEditor::_resetState()
	{
		for (auto& currBindingEntityInfo : mCurvesTable)
		{
			auto& bindingInfo = currBindingEntityInfo.second;

			if (auto serializationCallback = bindingInfo.mOnSerializeCurvePointsCallback)
			{
				serializationCallback(bindingInfo.mpCurve);
			}
		}

		mCurvesTable.clear();

		return RC_OK;
	}

	void CTrackSheetEditor::_initCurvesState()
	{
		for (auto& currBindingEntityInfo : mCurvesTable)
		{
			auto& bindingInfo = currBindingEntityInfo.second;

			if (auto serializationCallback = bindingInfo.mOnInitCurvePointsCallback)
			{
				serializationCallback(bindingInfo.mpCurve);
			}
		}
	}


	TDE2_API CTrackSheetEditor* CreateTrackSheetEditor(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CTrackSheetEditor, CTrackSheetEditor, result);
	}
}