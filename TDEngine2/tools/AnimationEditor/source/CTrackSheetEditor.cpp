#include "../include/CTrackSheetEditor.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	static const TVector2 DiscreteTrackButtonSize(10.0f, 20.0f);
	
	static const F32 GridVerticalOffsetMultiplier = 1e-3f;

	static const std::string KeyOperationsPopupMenuId = "KeyMenuWindow";



	CTrackSheetEditor::CTrackSheetEditor() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CTrackSheetEditor::Init(TPtr<IDesktopInputContext> pInputContext)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pInputContext)
		{
			return RC_INVALID_ARGS;
		}

		mpInputContext = pInputContext;

		mIsInitialized = true;

		return RC_OK;
	}


	static void ProcessInput(IImGUIContext& imguiContext, TPtr<IDesktopInputContext> pInputContext, TVector2& vertGridBounds)
	{
		//imguiContext.
		if (pInputContext->IsKey(E_KEYCODES::KC_LCONTROL))
		{
			const F32 scrollDelta = pInputContext->GetMouseShiftVec().z * GridVerticalOffsetMultiplier;

			vertGridBounds.x -= scrollDelta;
			vertGridBounds.y += scrollDelta;
		}
	}


	static void DrawCurvesSelector(CTrackSheetEditor::TCurveBindingsTable& curvesTable, IImGUIContext& imguiContext, std::string& currSelectedCurveId)
	{
		bool isOpened = true;
		
		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(100.0f, 100.0f),
			TVector2(100.0f, 100.0f),
		};

		if (imguiContext.BeginWindow("Curves", isOpened, params))
		{
			for (auto& currCurve : curvesTable)
			{
				if (imguiContext.SelectableItem(currCurve.first, currSelectedCurveId == currCurve.first))
				{
					currSelectedCurveId = currCurve.first;
				}
			}

			imguiContext.EndWindow();
		}
	}


	E_RESULT_CODE CTrackSheetEditor::Draw(const TVector2& frameSizes)
	{
		if (mOnDrawImpl)
		{
			mOnDrawImpl(frameSizes);
			mIsEditing = true;
		}

		ProcessInput(*mpImGUIContext, mpInputContext, mCurrGridYAxisBounds);
		DrawCurvesSelector(mCurvesTable, *mpImGUIContext, mCurrSelectedCurveId);

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
				CAnimationCurve::TKeyFrame value;
				value.mTime = currKey.mTime;

				if (getterAction(currKey, value))
				{
					pCurve->ReplacePoint(value);
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
						setterAction(*pKeySample, *pCurrPoint);
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


	/*!
		\brief The enum is used in the macro to retrieve offset for mUsedChannels value
	*/

	enum E_COMPONENT_TO_CHANNELS_FLAGS
	{
		CTCF_x = 0,
		CTCF_y = 1,
		CTCF_z = 2,
		CTCF_w = 3,
		CTCF_r = CTCF_x,
		CTCF_g = CTCF_y,
		CTCF_b = CTCF_z,
		CTCF_a = CTCF_w,
	};


#define TDE2_COMP_OFFSET(channel) CTCF_ ## channel

#define TDE2_TRACK_TO_CURVE_LAMBDA_IMPL(KeyFrameType, channel, valueAccess)		\
	[](const KeyFrameType& value, auto& outValue)								\
	{																			\
		outValue.mValue = valueAccess;											\
																				\
		outValue.mInTangent = value.mInTangents[TDE2_COMP_OFFSET(channel)];		\
		outValue.mOutTangent = value.mOutTangents[TDE2_COMP_OFFSET(channel)];	\
																				\
		return (value.mUsedChannels & (1 << TDE2_COMP_OFFSET(channel)));		\
	}

#define TDE2_FRAME_ACCESS(variable, channel) variable.mValue. channel
#define TDE2_TRACK_TO_CURVE_LAMBDA(KeyFrameType, channel) TDE2_TRACK_TO_CURVE_LAMBDA_IMPL(KeyFrameType, channel, TDE2_FRAME_ACCESS(value, channel))

#define TDE2_TRACK_TO_CURVE_LAMBDA_SIMPLE(KeyFrameType) TDE2_TRACK_TO_CURVE_LAMBDA_IMPL(KeyFrameType, x, value.mValue)


#define TDE2_CURVE_TO_TRACK_LAMBDA_IMPL(KeyFrameType, channel, valueAccess) \
	[](KeyFrameType& dest, const CAnimationCurve::TKeyFrame& value)			\
	{																		\
		dest.mUsedChannels |= (1 << TDE2_COMP_OFFSET(channel));				\
																			\
		dest.mInTangents[TDE2_COMP_OFFSET(channel)] = value.mInTangent;		\
		dest.mOutTangents[TDE2_COMP_OFFSET(channel)] = value.mOutTangent;	\
																			\
		valueAccess = value.mValue;											\
	}

#define TDE2_CURVE_TO_TRACK_LAMBDA(KeyFrameType, channel) TDE2_CURVE_TO_TRACK_LAMBDA_IMPL(KeyFrameType, channel, TDE2_FRAME_ACCESS(dest, channel))
#define TDE2_CURVE_TO_TRACK_LAMBDA_SIMPLE(KeyFrameType) TDE2_CURVE_TO_TRACK_LAMBDA_IMPL(KeyFrameType, x, dest.mValue)


	E_RESULT_CODE CTrackSheetEditor::VisitVector2Track(CVector2AnimationTrack* pTrack)
	{
		E_RESULT_CODE result = _resetState();
		if (RC_OK != result)
		{
			return result;
		}

		const TRectF32 trackBounds{ 0.0f, 0.0f, pTrack->GetOwner()->GetDuration(), 0.0f };

		/// \note Create new curves 
		mCurvesTable.emplace("x", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, TDE2_TRACK_TO_CURVE_LAMBDA(TVector2KeyFrame, x)),
			GenerateSerializePointsCallback(pTrack, TDE2_CURVE_TO_TRACK_LAMBDA(TVector2KeyFrame, x))});

		mCurvesTable.emplace("y", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, TDE2_TRACK_TO_CURVE_LAMBDA(TVector2KeyFrame, y)),
			GenerateSerializePointsCallback(pTrack, TDE2_CURVE_TO_TRACK_LAMBDA(TVector2KeyFrame, y)) });

		mOnDrawImpl = _generateDrawCallback(pTrack->GetOwner()->GetDuration(), pTrack);

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

		const TRectF32 trackBounds { 0.0f, 0.0f, pTrack->GetOwner()->GetDuration(), 0.0f };

		/// \note Create new curves 
		mCurvesTable.emplace("x", TTrack2CurveBindingInfo { CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) }, 
			GenerateInitCurveCallback(pTrack, TDE2_TRACK_TO_CURVE_LAMBDA(TVector3KeyFrame, x)),
			GenerateSerializePointsCallback(pTrack,TDE2_CURVE_TO_TRACK_LAMBDA(TVector3KeyFrame, x)) });

		mCurvesTable.emplace("y", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, TDE2_TRACK_TO_CURVE_LAMBDA(TVector3KeyFrame, y)),
			GenerateSerializePointsCallback(pTrack, TDE2_CURVE_TO_TRACK_LAMBDA(TVector3KeyFrame, y)) });

		mCurvesTable.emplace("z", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, TDE2_TRACK_TO_CURVE_LAMBDA(TVector3KeyFrame, z)),
			GenerateSerializePointsCallback(pTrack, TDE2_CURVE_TO_TRACK_LAMBDA(TVector3KeyFrame, z)) });

		mOnDrawImpl = _generateDrawCallback(pTrack->GetOwner()->GetDuration(), pTrack);

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

		const TRectF32 trackBounds{ 0.0f, 0.0f, pTrack->GetOwner()->GetDuration(), 0.0f };

		/// \note Create new curves 
		mCurvesTable.emplace("x", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, TDE2_TRACK_TO_CURVE_LAMBDA(TQuaternionKeyFrame, x)),
			GenerateSerializePointsCallback(pTrack, TDE2_CURVE_TO_TRACK_LAMBDA(TQuaternionKeyFrame, x)) });

		mCurvesTable.emplace("y", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, TDE2_TRACK_TO_CURVE_LAMBDA(TQuaternionKeyFrame, y)),
			GenerateSerializePointsCallback(pTrack, TDE2_CURVE_TO_TRACK_LAMBDA(TQuaternionKeyFrame, y)) });

		mCurvesTable.emplace("z", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, TDE2_TRACK_TO_CURVE_LAMBDA(TQuaternionKeyFrame, z)),
			GenerateSerializePointsCallback(pTrack, TDE2_CURVE_TO_TRACK_LAMBDA(TQuaternionKeyFrame, z)) });

		mCurvesTable.emplace("w", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, TDE2_TRACK_TO_CURVE_LAMBDA(TQuaternionKeyFrame, w)),
			GenerateSerializePointsCallback(pTrack, TDE2_CURVE_TO_TRACK_LAMBDA(TQuaternionKeyFrame, w)) });

		mOnDrawImpl = _generateDrawCallback(pTrack->GetOwner()->GetDuration(), pTrack);

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

		const TRectF32 trackBounds{ 0.0f, 0.0f, pTrack->GetOwner()->GetDuration(), 0.0f };

		/// \note Create new curves 
		mCurvesTable.emplace("r", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, TDE2_TRACK_TO_CURVE_LAMBDA(TColorKeyFrame, r)),
			GenerateSerializePointsCallback(pTrack, TDE2_CURVE_TO_TRACK_LAMBDA(TColorKeyFrame, r)) });

		mCurvesTable.emplace("g", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, TDE2_TRACK_TO_CURVE_LAMBDA(TColorKeyFrame, g)),
			GenerateSerializePointsCallback(pTrack, TDE2_CURVE_TO_TRACK_LAMBDA(TColorKeyFrame, g)) });

		mCurvesTable.emplace("b", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, TDE2_TRACK_TO_CURVE_LAMBDA(TColorKeyFrame, b)),
			GenerateSerializePointsCallback(pTrack, TDE2_CURVE_TO_TRACK_LAMBDA(TColorKeyFrame, b)) });

		mCurvesTable.emplace("a", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, TDE2_TRACK_TO_CURVE_LAMBDA(TColorKeyFrame, a)),
			GenerateSerializePointsCallback(pTrack, TDE2_CURVE_TO_TRACK_LAMBDA(TColorKeyFrame, a)) });

		mOnDrawImpl = _generateDrawCallback(pTrack->GetOwner()->GetDuration(), pTrack);

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

		const TRectF32 trackBounds{ 0.0f, 0.0f, pTrack->GetOwner()->GetDuration(), 0.0f };

		/// \note Create new curves 
		mCurvesTable.emplace("value", TTrack2CurveBindingInfo{ CScopedPtr<CAnimationCurve> { CreateAnimationCurve(trackBounds, result) },
			GenerateInitCurveCallback(pTrack, TDE2_TRACK_TO_CURVE_LAMBDA_SIMPLE(TFloatKeyFrame)),
			GenerateSerializePointsCallback(pTrack, TDE2_CURVE_TO_TRACK_LAMBDA_SIMPLE(TFloatKeyFrame)) });

		mPreSerializeAction = GeneratePreSerializeCallback(mCurvesTable, pTrack);

		mOnDrawImpl = _generateDrawCallback(pTrack->GetOwner()->GetDuration(), pTrack);

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

				mpImGUIContext->TextField("Event Id:", eventId, [this, pTrack](auto&& value)
				{
					if (auto pKeySample = pTrack->GetKey(mCurrSelectedSampleId))
					{
						pKeySample->mValue = value;
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

		mCurrGridYAxisBounds = TVector2((std::numeric_limits<F32>::max)(), -(std::numeric_limits<F32>::max)());

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

	CTrackSheetEditor::TCurveDrawCallback CTrackSheetEditor::_generateDrawCallback(F32 trackDuration, IAnimationTrack* pTrack)
	{
		return [this, trackDuration, pTrack](const TVector2& frameSizes)
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

			F32& minY = mCurrGridYAxisBounds.x;
			F32& maxY = mCurrGridYAxisBounds.y;

			/// \note Find common boundaries for all curves
			for (auto&& currCurveBindingInfo : mCurvesTable)
			{
				auto&& currBounds = currCurveBindingInfo.second.mpCurve->GetBounds();

				minY = std::min<F32>(minY, currBounds.y);
				maxY = std::max<F32>(maxY, currBounds.y + currBounds.height);
			}

			const TRectF32 gridBounds(0.0f, minY, trackDuration, CMathUtils::Abs(maxY - minY));

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
				curveParams.mCurveColor = ((mCurvesTable.size() < 2) ? curveColors[2] : (mCurrSelectedCurveId != currCurveBindingInfo.first ? curveColors.back() : curveColors[i]));
				curveParams.mShouldIgnoreInput = mCurrSelectedCurveId != currCurveBindingInfo.first;
				curveParams.mUseCustomGridBounds = true;
				curveParams.mGridBounds = gridBounds;
				curveParams.mIsTangentControlsEnabled = pTrack ? (pTrack->GetInterpolationMode() == E_ANIMATION_INTERPOLATION_MODE_TYPE::CUBIC) : false;
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


	TDE2_API CTrackSheetEditor* CreateTrackSheetEditor(TPtr<IDesktopInputContext> pInputContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CTrackSheetEditor, CTrackSheetEditor, result, pInputContext);
	}
}

#endif