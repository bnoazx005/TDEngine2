#include "../include/CTrackSheetEditor.h"


namespace TDEngine2
{
	static const TVector2 DiscreteTrackButtonSize(10.0f, 20.0f);

	static const std::string KeyOperationsPopupMenuId = "KeyMenuWindow";



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

		mOnDrawImpl = _generateDrawCallback();

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

		mOnDrawImpl = _generateDrawCallback();

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

		mOnDrawImpl = _generateDrawCallback();

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

		mOnDrawImpl = _generateDrawCallback();

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


	template <typename T>
	TAnimationTrackKeyId DrawDiscreteTrackSamples(IImGUIContext* pImGUIContext, T* pTrack, const TVector2& frameSizes)
	{
		TAnimationTrackKeyId selectedSampleKeyHandle = TAnimationTrackKeyId::Invalid;

		auto&& cursorPos = pImGUIContext->GetCursorScreenPos();

		pImGUIContext->DrawLine(cursorPos + TVector2(0.0f, frameSizes.y * 0.5f), cursorPos + TVector2(frameSizes.x, frameSizes.y * 0.5f), TColorUtils::mGray, 1.0f);

		const F32 pixelsPerSecond = frameSizes.x / pTrack->GetOwner()->GetDuration();

		U32 id = 0;

		/// \note Implement creation of a new events
		if (pImGUIContext->IsMouseDoubleClicked(0))
		{
			auto handle = pTrack->CreateKey(CMathUtils::Clamp(0.0f, frameSizes.x, pImGUIContext->GetMousePosition().x - cursorPos.x) / pixelsPerSecond);
			TDE2_ASSERT(TAnimationTrackKeyId::Invalid != handle);
		}

		for (auto&& currKeySample : pTrack->GetKeys())
		{
			const TVector2 buttonPosition = cursorPos + TVector2(currKeySample.mTime * pixelsPerSecond, frameSizes.y * 0.5f) - DiscreteTrackButtonSize * 0.5f;

			pImGUIContext->DrawRect(TRectF32(buttonPosition.x, buttonPosition.y, DiscreteTrackButtonSize.x, DiscreteTrackButtonSize.y), TColorUtils::mGreen, true);
			pImGUIContext->DrawText(buttonPosition - TVector2(0.0f, DiscreteTrackButtonSize.y), TColorUtils::mWhite, currKeySample.mValue);

			pImGUIContext->DisplayIDGroup((1 << 16) | id, [pImGUIContext, &currKeySample, &buttonPosition, pixelsPerSecond, cursorPos, pTrack]
			{
				auto&& pos = pImGUIContext->GetCursorScreenPos();

				pImGUIContext->SetCursorScreenPos(buttonPosition);
				pImGUIContext->Button(Wrench::StringUtils::GetEmptyStr(), DiscreteTrackButtonSize, nullptr, true);
				pImGUIContext->SetCursorScreenPos(pos);

				/// \note Keyd dragging logic
				if (pImGUIContext->IsItemActive())
				{
					auto&& mousePos = pImGUIContext->GetMousePosition();

					if (pImGUIContext->IsMouseDragging(0))
					{
						pTrack->UpdateKeyTime(pTrack->GetKeyHandleByTime(currKeySample.mTime), CMathUtils::Clamp(0.0f, pTrack->GetOwner()->GetDuration(), (mousePos.x - cursorPos.x) / pixelsPerSecond));
					}
				}
			});

			if (pImGUIContext->IsItemHovered() && pImGUIContext->IsMouseClicked(1))
			{
				pImGUIContext->ShowModalWindow(KeyOperationsPopupMenuId);
				selectedSampleKeyHandle = pTrack->GetKeyHandleByTime(currKeySample.mTime);
			}

			++id;
		}

		return selectedSampleKeyHandle;
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

		mOnDrawImpl = [this, pTrack](const TVector2& frameSizes)
		{
			auto newSelectedTrackHandle = DrawDiscreteTrackSamples(mpImGUIContext, pTrack, frameSizes);
			if (TAnimationTrackKeyId::Invalid != newSelectedTrackHandle)
			{
				mCurrSelectedSampleId = newSelectedTrackHandle;
			}

			/// \note Context menu
			if ((TAnimationTrackKeyId::Invalid != mCurrSelectedSampleId) && mpImGUIContext->BeginModalWindow(KeyOperationsPopupMenuId))
			{
				std::string eventId = pTrack->GetKey(mCurrSelectedSampleId)->mValue;

				mpImGUIContext->TextField("Event Id:", eventId, [this, &eventId, pTrack]
				{
					if (auto pKeySample = pTrack->GetKey(mCurrSelectedSampleId))
					{
						pKeySample->mValue = eventId;
					}

					mCurrSelectedSampleId = TAnimationTrackKeyId::Invalid;
					mpImGUIContext->CloseCurrentModalWindow();
				});

				if (mpImGUIContext->Button("Remove Key", TVector2(50.0f, 25.0f)))
				{
					E_RESULT_CODE result = pTrack->RemoveKey(mCurrSelectedSampleId);
					TDE2_ASSERT(RC_OK == result);

					mCurrSelectedSampleId = TAnimationTrackKeyId::Invalid;
					mpImGUIContext->CloseCurrentModalWindow();
				}

				mpImGUIContext->EndModalWindow();
			}
		};

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

	CTrackSheetEditor::TCurveDrawCallback CTrackSheetEditor::_generateDrawCallback()
	{
		return [this](const TVector2& frameSizes)
		{
			U8 i = 0;

			static const std::array<TColor32F, 4> curveColors
			{
				TColorUtils::mRed,
				TColorUtils::mGreen,
				TColorUtils::mBlue,
				TColorUtils::mGray,
			};

			TAnimationCurveEditorParams curveParams;

			for (auto&& currCurveBindingInfo : mCurvesTable)
			{
				auto& curveInfo = currCurveBindingInfo.second;

				if (mCurrSelectedCurveId.empty())
				{
					mCurrSelectedCurveId = currCurveBindingInfo.first;
				}

				curveParams.mFrameWidth = frameSizes.x;
				curveParams.mFrameHeight = frameSizes.y;
				curveParams.mIsGridVisible = !i; /// \note Draw the grid only for the first curve
				curveParams.mIsBackgroundVisible = false;
				curveParams.mCurveColor = ((mCurvesTable.size() < 2) ? curveColors[2] : (mCurrSelectedCurveId != currCurveBindingInfo.first ? curveColors[3] : curveColors[i]));
				curveParams.mShouldIgnoreInput = mCurrSelectedCurveId != currCurveBindingInfo.first;
				curveParams.mOnCurveClickedCallback = [id = currCurveBindingInfo.first, this]
				{
					mCurrSelectedCurveId = id;
				};

				CAnimationCurveEditorWindow::DrawCurveEditor(mpImGUIContext, curveParams, curveInfo.mpCurve.Get());
				++i;
			}
		};
	}


	TDE2_API CTrackSheetEditor* CreateTrackSheetEditor(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CTrackSheetEditor, CTrackSheetEditor, result);
	}
}