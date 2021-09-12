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


	template <typename T, typename TGetterType>
	static CTrackSheetEditor::TCurveBindingCallback GenerateInitCurveCallback(T* pTrack, const TGetterType& getterAction)
	{
		return [pTrack, &getterAction](const CScopedPtr<CAnimationCurve>& pCurve)
		{
			for (auto&& currKey : pTrack->GetKeys())
			{
				F32 value = 0.0f;

				if (getterAction(currKey, value))
				{
					pCurve->ReplacePoint(CAnimationCurve::TKeyFrame{ currKey.mTime, value });
				}
			}
		};
	}


	template <typename T, typename TSetterType>
	static CTrackSheetEditor::TCurveBindingCallback GenerateSerializePointsCallback(T* pTrack, const TSetterType& setterAction)
	{
		return [pTrack, &setterAction](const CScopedPtr<CAnimationCurve>& pCurve)
		{
			const F32 trackDuration = pTrack->GetOwner()->GetDuration();

			for (U32 i = 0; i < pCurve->GetPointsCount(); ++i)
			{
				if (auto&& pCurrPoint = pCurve->GetPoint(i))
				{
					if (auto pKeySample = pTrack->GetKey(pTrack->CreateKey(CMathUtils::Clamp(0.0f, trackDuration, pCurrPoint->mTime))))
					{
						setterAction(*pKeySample, pCurrPoint->mValue);
					}					
				}
			}
		};
	}


	template <typename T>
	static CTrackSheetEditor::TActionCallback GeneratePreSerializeCallback(CTrackSheetEditor::TCurveBindingsTable& curvesTable, T* pTrack)
	{
		return [&curvesTable, pTrack]
		{
			pTrack->RemoveAllKeys();

			const F32 trackDuration = pTrack->GetOwner()->GetDuration();

			constexpr U8 initialUsedChannelsMask = 0x0;

			/// \note Create new keys that are met in all curves
			for (auto&& currCurveInfo : curvesTable)
			{
				if (auto&& pCurve = currCurveInfo.second.mpCurve.Get())
				{
					for (U32 i = 0; i < pCurve->GetPointsCount(); ++i)
					{
						if (auto&& pCurrPoint = pCurve->GetPoint(i))
						{
							auto handle = pTrack->CreateKey(CMathUtils::Clamp(0.0f, trackDuration, pCurrPoint->mTime), &initialUsedChannelsMask);
							TDE2_ASSERT(TAnimationTrackKeyId::Invalid != handle);
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
			GenerateInitCurveCallback(pTrack, [](const TVector2KeyFrame& value, F32& outValue) { outValue = value.mValue.x; return (value.mUsedChannels & 0x1); }),
			GenerateSerializePointsCallback(pTrack, [](TVector2KeyFrame& dest, F32 value) { dest.mUsedChannels |= 0x1; dest.mValue.x = value; }) });

		mCurvesTable.emplace("y", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, [](const TVector2KeyFrame& value, F32& outValue) { outValue = value.mValue.y; return (value.mUsedChannels & 0x2); }),
			GenerateSerializePointsCallback(pTrack, [](TVector2KeyFrame& dest, F32 value) { dest.mUsedChannels |= 0x2; dest.mValue.y = value; }) });

		mOnDrawImpl = _generateDrawCallback(pTrack->GetOwner()->GetDuration());

		mPreSerializeAction = GeneratePreSerializeCallback(mCurvesTable, pTrack);

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
			GenerateInitCurveCallback(pTrack, [](const TVector3KeyFrame& value, F32& outValue) { outValue = value.mValue.x; return (value.mUsedChannels & 0x1); }),
			GenerateSerializePointsCallback(pTrack, [](TVector3KeyFrame& dest, F32 value) { dest.mUsedChannels |= 0x1; dest.mValue.x = value; }) });

		mCurvesTable.emplace("y", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, [](const TVector3KeyFrame& value, F32& outValue) { outValue = value.mValue.y; return (value.mUsedChannels & 0x2); }),
			GenerateSerializePointsCallback(pTrack, [](TVector3KeyFrame& dest, F32 value) { dest.mUsedChannels |= 0x2; dest.mValue.y = value; }) });

		mCurvesTable.emplace("z", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, [](const TVector3KeyFrame& value, F32& outValue) { outValue = value.mValue.z; return (value.mUsedChannels & 0x4); }),
			GenerateSerializePointsCallback(pTrack, [](TVector3KeyFrame& dest, F32 value) { dest.mUsedChannels |= 0x4; dest.mValue.z = value; }) });

		mOnDrawImpl = _generateDrawCallback(pTrack->GetOwner()->GetDuration());

		mPreSerializeAction = GeneratePreSerializeCallback(mCurvesTable, pTrack);

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
			GenerateInitCurveCallback(pTrack, [](const TQuaternionKeyFrame& value, F32& outValue) { outValue = value.mValue.x; return (value.mUsedChannels & 0x1); }),
			GenerateSerializePointsCallback(pTrack, [](TQuaternionKeyFrame& dest, F32 value) { dest.mUsedChannels |= 0x1; dest.mValue.x = value; }) });

		mCurvesTable.emplace("y", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, [](const TQuaternionKeyFrame& value, F32& outValue) { outValue = value.mValue.y; return (value.mUsedChannels & 0x2); }),
			GenerateSerializePointsCallback(pTrack, [](TQuaternionKeyFrame& dest, F32 value) { dest.mUsedChannels |= 0x2; dest.mValue.y = value; }) });

		mCurvesTable.emplace("z", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, [](const TQuaternionKeyFrame& value, F32& outValue) { outValue = value.mValue.z; return (value.mUsedChannels & 0x4); }),
			GenerateSerializePointsCallback(pTrack, [](TQuaternionKeyFrame& dest, F32 value) { dest.mUsedChannels |= 0x4; dest.mValue.z = value; }) });

		mCurvesTable.emplace("w", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, [](const TQuaternionKeyFrame& value, F32& outValue) { outValue = value.mValue.w; return (value.mUsedChannels & 0x8); }),
			GenerateSerializePointsCallback(pTrack, [](TQuaternionKeyFrame& dest, F32 value) { dest.mUsedChannels |= 0x8; dest.mValue.w = value; }) });

		mOnDrawImpl = _generateDrawCallback(pTrack->GetOwner()->GetDuration());

		mPreSerializeAction = GeneratePreSerializeCallback(mCurvesTable, pTrack);

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
			GenerateInitCurveCallback(pTrack, [](const TColorKeyFrame& value, F32& outValue) { outValue = value.mValue.r; return (value.mUsedChannels & 0x1); }),
			GenerateSerializePointsCallback(pTrack, [](TColorKeyFrame& dest, F32 value) { dest.mUsedChannels |= 0x1; dest.mValue.r = value; }) });

		mCurvesTable.emplace("g", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, [](const TColorKeyFrame& value, F32& outValue) { outValue = value.mValue.g; return (value.mUsedChannels & 0x2); }),
			GenerateSerializePointsCallback(pTrack, [](TColorKeyFrame& dest, F32 value) { dest.mUsedChannels |= 0x2; dest.mValue.g = value; }) });

		mCurvesTable.emplace("b", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, [](const TColorKeyFrame& value, F32& outValue) { outValue = value.mValue.b; return (value.mUsedChannels & 0x4); }),
			GenerateSerializePointsCallback(pTrack, [](TColorKeyFrame& dest, F32 value) { dest.mUsedChannels |= 0x4; dest.mValue.b = value; }) });

		mCurvesTable.emplace("a", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, [](const TColorKeyFrame& value, F32& outValue) { outValue = value.mValue.a; return (value.mUsedChannels & 0x8); }),
			GenerateSerializePointsCallback(pTrack, [](TColorKeyFrame& dest, F32 value) { dest.mUsedChannels |= 0x8; dest.mValue.a = value; }) });

		mOnDrawImpl = _generateDrawCallback(pTrack->GetOwner()->GetDuration());

		mPreSerializeAction = GeneratePreSerializeCallback(mCurvesTable, pTrack);

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
			GenerateInitCurveCallback(pTrack, [](const TFloatKeyFrame& value, F32& outValue) { outValue = value.mValue; return (value.mUsedChannels & 0x1); }),
			[pTrack](const CScopedPtr<CAnimationCurve>& pCurve)
			{
			} });

		mPreSerializeAction = GeneratePreSerializeCallback(mCurvesTable, pTrack);

		mOnDrawImpl = _generateDrawCallback(pTrack->GetOwner()->GetDuration());

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
			pImGUIContext->DrawText(buttonPosition - TVector2(0.0f, DiscreteTrackButtonSize.y), TColorUtils::mWhite, ToString(currKeySample.mValue));

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
				const I32 currValue = pTrack->GetKey(mCurrSelectedSampleId)->mValue;
				I32 sampleValue = currValue;

				mpImGUIContext->BeginHorizontal();
				mpImGUIContext->Label("Value: ");
				mpImGUIContext->IntField(Wrench::StringUtils::GetEmptyStr(), sampleValue);
				mpImGUIContext->EndHorizontal();

				if (sampleValue != currValue)
				{
					if (auto pKeySample = pTrack->GetKey(mCurrSelectedSampleId))
					{
						pKeySample->mValue = sampleValue;
					}

					mCurrSelectedSampleId = TAnimationTrackKeyId::Invalid;
					mpImGUIContext->CloseCurrentModalWindow();
				}

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

	E_RESULT_CODE CTrackSheetEditor::VisitBooleanTrack(CBooleanAnimationTrack* pTrack)
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
				const bool currValue = pTrack->GetKey(mCurrSelectedSampleId)->mValue;
				bool sampleValue = currValue;

				mpImGUIContext->BeginHorizontal();
				mpImGUIContext->Label("Value: ");
				mpImGUIContext->Checkbox(Wrench::StringUtils::GetEmptyStr(), sampleValue);
				mpImGUIContext->EndHorizontal();

				if (sampleValue != currValue)
				{
					if (auto pKeySample = pTrack->GetKey(mCurrSelectedSampleId))
					{
						pKeySample->mValue = sampleValue;
					}

					mCurrSelectedSampleId = TAnimationTrackKeyId::Invalid;
					mpImGUIContext->CloseCurrentModalWindow();
				}

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
		if (mPreSerializeAction && !mCurvesTable.empty())
		{
			mPreSerializeAction();
		}

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

	CTrackSheetEditor::TCurveDrawCallback CTrackSheetEditor::_generateDrawCallback(F32 trackDuration)
	{
		return [this, trackDuration](const TVector2& frameSizes)
		{
			U8 i = 0;

			static const std::array<TColor32F, 5> curveColors
			{
				TColorUtils::mRed,
				TColorUtils::mGreen,
				TColorUtils::mBlue,
				TColorUtils::mWhite,
				TColorUtils::mGray,
			};

			TAnimationCurveEditorParams curveParams;

			TRectF32 gridBounds(0.0f, (std::numeric_limits<F32>::max)(), trackDuration, -(std::numeric_limits<F32>::max)());

			/// \note Find common boundaries for all curves
			for (auto&& currCurveBindingInfo : mCurvesTable)
			{
				auto&& currBounds = currCurveBindingInfo.second.mpCurve->GetBounds();

				gridBounds.y      = std::min<F32>(gridBounds.y, currBounds.y);
				gridBounds.height = std::max<F32>(gridBounds.height, currBounds.height);
			}

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
				curveParams.mCurveColor = ((mCurvesTable.size() < 2) ? curveColors[2] : (mCurrSelectedCurveId != currCurveBindingInfo.first ? curveColors[4] : curveColors[i]));
				curveParams.mShouldIgnoreInput = mCurrSelectedCurveId != currCurveBindingInfo.first;
				curveParams.mUseCustomGridBounds = true;
				curveParams.mGridBounds = gridBounds;
				curveParams.mOnCurveClickedCallback = [id = currCurveBindingInfo.first, this]
				{
					mCurrSelectedCurveId = id;
				};

				if (CAnimationCurve* pCurve = curveInfo.mpCurve.Get())
				{
					CAnimationCurveEditorWindow::DrawCurveEditor(mpImGUIContext, curveParams, pCurve);
				}

				++i;
			}
		};
	}


	TDE2_API CTrackSheetEditor* CreateTrackSheetEditor(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CTrackSheetEditor, CTrackSheetEditor, result);
	}
}