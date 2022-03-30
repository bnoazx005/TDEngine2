#include "../include/CAnimationEditorWindow.h"
#include "../../include/metadata.h"


namespace TDEngine2
{
	const std::string CAnimationEditorWindow::mAddPropertyWindowId = "Add Property ...";


	CAnimationEditorWindow::CAnimationEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CAnimationEditorWindow::Init(TPtr<IResourceManager> pResourceManager, TPtr<IWorld> pWorld, TPtr<IDesktopInputContext> pInputContext)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pResourceManager || !pWorld || !pInputContext)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;
		mpWorld = pWorld;
		mpInputContext = pInputContext;

		mCurrAnimatedEntity = TEntityId::Invalid;

		E_RESULT_CODE result = RC_OK;

		mpTrackSheetEditor = CreateTrackSheetEditor(pInputContext, result);

		if (RC_OK != result)
		{
			return result;
		}

		mIsInitialized = true;
		mIsVisible = true;

		return RC_OK;
	}

	void CAnimationEditorWindow::SetAnimationResourceHandle(TResourceId handle)
	{
		mCurrAnimationResourceHandle = handle;
		mpCurrAnimationClip = mpResourceManager->GetResource<IAnimationClip>(mCurrAnimationResourceHandle);
	}

	void CAnimationEditorWindow::SetAnimatedEntityId(TEntityId entity)
	{
		mCurrAnimatedEntity = entity;
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

			bool isAnimationPlaying = false;

			if (auto pEntity = mpWorld->FindEntity(mCurrAnimatedEntity))
			{
				if (auto pAnimationContainer = pEntity->GetComponent<CAnimationContainerComponent>())
				{
					isAnimationPlaying = pAnimationContainer->IsPlaying();
				}
			}

			mpImGUIContext->Button("*##Record", buttonSize);

			auto rewindAnimationFunctor = [this](float timeStep)
			{
				return [this, timeStep]
				{
					if (auto pEntity = mpWorld->FindEntity(mCurrAnimatedEntity))
					{
						if (auto pAnimationContainer = pEntity->GetComponent<CAnimationContainerComponent>())
						{
							pAnimationContainer->SetTime(pAnimationContainer->GetTime() + timeStep);
						}
					}
				};
			};

			mpImGUIContext->Button("<|##BackStep", buttonSize, rewindAnimationFunctor(-1.0f));

			mpImGUIContext->Button(isAnimationPlaying ? "Stop" : "Play", TVector2(40.0f, 20.0f), [this]
			{
				if (auto pEntity = mpWorld->FindEntity(mCurrAnimatedEntity))
				{
					if (auto pAnimationContainer = pEntity->GetComponent<CAnimationContainerComponent>())
					{
						if (pAnimationContainer->IsPlaying())
						{
							pAnimationContainer->SetStoppedFlag(true);
						}
						else
						{
							pAnimationContainer->Play();
						}
					}
				}
			});

			mpImGUIContext->Button("|>##ForwardStep", buttonSize, rewindAnimationFunctor(1.0f));

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

						if ((CMathUtils::Abs(duration - mLastClipDuration) > FloatEpsilon) && (TAnimationTrackId::Invalid != mSelectedTrackId))
						{
							if (auto pTrack = mpCurrAnimationClip->GetTrack<IAnimationTrack>(mSelectedTrackId))
							{
								mpTrackSheetEditor->SetImGUIContext(mpImGUIContext);
								pTrack->AssignTrackForEditing(mpTrackSheetEditor.Get());
							}

							mLastClipDuration = duration;
						}
					});
				});
			}

			/// \note Check whether any track exists or not in the clip
			if (mpCurrAnimationClip->GetTracksCount() > 0)
			{
				const bool prevDopeSheetModeState = mIsDopeSheetModeEnabled;

				mpImGUIContext->Label("Dope Sheet");
				mpImGUIContext->Checkbox("##DopeSheetMode", mIsDopeSheetModeEnabled);

				mHasEditModeBeenChanged = (prevDopeSheetModeState != mIsDopeSheetModeEnabled);
			}

			mpImGUIContext->EndHorizontal();

			mpImGUIContext->EndChildWindow();
		}
	}

	void CAnimationEditorWindow::_drawTimelineEditorGroup()
	{
		static F32 verticalSeparatorPos = mpImGUIContext->GetWindowWidth() * 0.3f;

		mpImGUIContext->SetCursorScreenPos(mpImGUIContext->GetCursorScreenPos() + TVector2(0.0f, 10.0f));
		mpImGUIContext->VerticalSeparator(verticalSeparatorPos,
			std::bind(&CAnimationEditorWindow::_drawTracksHierarchy, this, std::placeholders::_1), 
			std::bind(&CAnimationEditorWindow::_drawTimelineEditor, this, std::placeholders::_1));
	}


	/// \todo Move the method into some place where all the utilities for bindings will be stored
	static std::tuple<std::string, std::string> GetTrackInfoFromBinding(IWorld* pWorld, TEntityId currAnimatedEntity, const std::string& propertyBinding)
	{
		static const std::string InvalidEntity = "None";

		std::string binding = Wrench::StringUtils::RemoveAllWhitespaces(propertyBinding);

		std::string::size_type pos = 0;

		// \note If there are child appearances in the path go down into the hierarchy
		CEntity* pCurrEntity = pWorld->FindEntity(currAnimatedEntity);
		if (!pCurrEntity || (binding == Wrench::StringUtils::GetEmptyStr()))
		{
			return { InvalidEntity, Wrench::StringUtils::GetEmptyStr() };
		}

		auto&& hierarchy = Wrench::StringUtils::Split(binding, "/");
		for (auto it = hierarchy.cbegin(); it != std::prev(hierarchy.cend()); it++)
		{
			CTransform* pTransform = pCurrEntity->GetComponent<CTransform>();

			bool hasChildFound = false;

			for (TEntityId childEntityId : pTransform->GetChildren())
			{
				if (CEntity* pChildEntity = pWorld->FindEntity(childEntityId))
				{
					if (pChildEntity->GetName() == *it)
					{
						pCurrEntity = pChildEntity;
						hasChildFound = true;
						break;
					}
				}
			}

			if (!hasChildFound)
			{
				return { InvalidEntity, Wrench::StringUtils::GetEmptyStr() };
			}
		}

		// \note Check whether the component with given identifier exist or not
		const std::string& componentBinding = hierarchy.back();

		pos = componentBinding.find_first_of('.');
		if (pos == std::string::npos)
		{
			return { InvalidEntity, Wrench::StringUtils::GetEmptyStr() };
		}

		return { pCurrEntity->GetName(), componentBinding };
	}


	static void DisplaySetSampleModeMenuGroup(IImGUIContext& imguiContext, IAnimationTrack* pTrack)
	{
		imguiContext.MenuGroup("Sample Mode", [pTrack](IImGUIContext& imgui)
		{
			for (auto&& enumField : Meta::EnumTrait<E_ANIMATION_INTERPOLATION_MODE_TYPE>::GetFields())
			{
				imgui.MenuItem(enumField.name, Wrench::StringUtils::GetEmptyStr(), [pTrack, value = enumField.value]{ pTrack->SetInterpolationMode(value); });
			}
		});
	}


	void CAnimationEditorWindow::_drawTracksHierarchy(F32 blockWidth)
	{
		mpImGUIContext->SetCursorScreenPos(mpImGUIContext->GetCursorScreenPos() + TVector2(0.0f, 25.0f));

		mpImGUIContext->BeginChildWindow("##TracksHierarchyWidget", TVector2(blockWidth - 10.0f, mpImGUIContext->GetWindowHeight() * 0.65f));
		{
			mpCurrAnimationClip->ForEachTrack([this](TAnimationTrackId trackId, IAnimationTrack* pTrack)
			{
				const std::string& propertyBinding = pTrack->GetPropertyBinding();

				std::string entityName, bindingName;
				std::tie(entityName, bindingName) = GetTrackInfoFromBinding(mpWorld.Get(), mCurrAnimatedEntity, propertyBinding);

				if (mUsedPropertyBindings.find(propertyBinding) == mUsedPropertyBindings.cend())
				{
					mUsedPropertyBindings.insert(propertyBinding);
				}

				auto prevSelectedTrackId = mSelectedTrackId;

				if (TAnimationTrackId::Invalid == mSelectedTrackId)
				{
					mSelectedTrackId = trackId;
				}

				/// \todo For events track there should be a unique identifier
				if (mpImGUIContext->SelectableItem(Wrench::StringUtils::Format("{0}: {1}", entityName, bindingName), mSelectedTrackId == trackId))
				{					
					mSelectedTrackId = trackId;
				}

				if (prevSelectedTrackId != mSelectedTrackId)
				{
					mHasEditModeBeenChanged = true;
				}

				/// \todo Remove the selected track
				if (mSelectedTrackId == trackId)
				{
					mpImGUIContext->DisplayContextMenu("TracksOperations", [this, trackId, propertyBinding, pTrack](IImGUIContext& imguiContext)
					{
						imguiContext.MenuItem("Remove Track", "Del", [this, trackId, propertyBinding]
						{
							mUsedPropertyBindings.erase(propertyBinding);

							if (mpCurrAnimationClip)
							{
								E_RESULT_CODE result = mpCurrAnimationClip->RemoveTrack(trackId);
								TDE2_ASSERT(RC_OK == result);
							}

							mSelectedTrackId = TAnimationTrackId::Invalid;
						});

						DisplaySetSampleModeMenuGroup(imguiContext, pTrack);
					});

					if (TAnimationTrackId::Invalid == mSelectedTrackId)
					{
						return false;
					}
				}

				return true;
			});

			mTimelineScrollPosition = mpImGUIContext->GetScrollPosition();
		}
		mpImGUIContext->EndChildWindow();

		mpImGUIContext->BeginChildWindow("##TracksHierarchyToolbarWidget", TVector2(blockWidth - 10.0f, 35.0f));
		{
			_drawPropertyBindingsWindow();

			const TVector2 buttonSizes(mpImGUIContext->GetWindowWidth() * 0.45f, 25.0f);

			mpImGUIContext->BeginHorizontal();
			{
				if (mpImGUIContext->Button("Add Property", buttonSizes))
				{
					mpImGUIContext->ShowModalWindow(mAddPropertyWindowId);
				}

				if (!mpCurrAnimationClip->GetEventTrack())
				{
					if (mpImGUIContext->Button("Add Events Track", buttonSizes))
					{
						_onAddEventsTrackButtonHandler();
					}
				}				
			}
			mpImGUIContext->EndHorizontal();
		}
		mpImGUIContext->EndChildWindow();
	}

	void CAnimationEditorWindow::_onAddEventsTrackButtonHandler()
	{
		if (!mpCurrAnimationClip)
		{
			TDE2_ASSERT(false);
			return;
		}

		auto handle = mpCurrAnimationClip->CreateTrack<CEventAnimationTrack>("events");
		TDE2_ASSERT(TAnimationTrackId::Invalid != handle);
	}

	void CAnimationEditorWindow::_drawTimelineEditor(F32 blockWidth)
	{
		mpImGUIContext->BeginChildWindow("##TracksTimelineWidget", TVector2(blockWidth - 10.0f, mpImGUIContext->GetWindowHeight() * 0.8f));
		{
			F32 playbackTime = 0.0f;

			if (TEntityId::Invalid != mCurrAnimatedEntity)
			{
				if (auto pEntity = mpWorld->FindEntity(mCurrAnimatedEntity))
				{
					if (auto pAnimationContainer = pEntity->GetComponent<CAnimationContainerComponent>())
					{
						playbackTime = pAnimationContainer->GetTime();
					}
				}
			}

			mpImGUIContext->SetItemWidth(mpImGUIContext->GetWindowWidth(), [&playbackTime, this]
			{
				mpImGUIContext->FloatSlider("##Cursor", playbackTime, 0.0f, mpCurrAnimationClip ? mpCurrAnimationClip->GetDuration() : 0.0f, [&playbackTime, this] 
				{
					if (TEntityId::Invalid != mCurrAnimatedEntity)
					{
						if (auto pEntity = mpWorld->FindEntity(mCurrAnimatedEntity))
						{
							if (auto pAnimationContainer = pEntity->GetComponent<CAnimationContainerComponent>())
							{
								pAnimationContainer->SetTime(playbackTime);
							}
						}
					}
				});
			});

			_drawTimelineSheetWidget(playbackTime);
		}
		mpImGUIContext->EndChildWindow();
	}

	void CAnimationEditorWindow::_drawTimelineSheetWidget(F32 currPlaybackTime)
	{
		TVector2 cursorPos = mpImGUIContext->GetCursorScreenPos();

		const F32 duration = (mpCurrAnimationClip ? mpCurrAnimationClip->GetDuration() : 1.0f);
		const F32 timelineWidth = mpImGUIContext->GetWindowWidth();
		const F32 timelineHeight = mpImGUIContext->GetWindowHeight() * 0.95f;
		const F32 pixelsPerSecond = timelineWidth / duration;

		mpImGUIContext->DrawRect(TRectF32(cursorPos.x, cursorPos.y, timelineWidth, timelineHeight), TColor32F(0.2f, 0.2f, 0.2f, 1.0f), true, 1.0f);

		/// \note Draw vertical lines which determine seconds
		for (F32 t = 0.0f; CMathUtils::IsLessOrEqual(t, duration); t += 1.0f)
		{
			mpImGUIContext->DrawLine(cursorPos + TVector2(t * pixelsPerSecond, 0.0f), cursorPos + TVector2(t * pixelsPerSecond, timelineHeight), TColor32F(0.25f, 0.25f, 0.25f, 1.0f));
		}

		/// \note Draw tracks
		if (mIsDopeSheetModeEnabled)
		{
			if (mpTrackSheetEditor && mpTrackSheetEditor->IsEditing())
			{
				mpTrackSheetEditor->Reset(); /// \note Save data back into the tracks
			}

			_drawDopesheetTracks(cursorPos, timelineWidth, timelineHeight, pixelsPerSecond);
		}
		else
		{
			if (mHasEditModeBeenChanged)
			{
				if (auto pTrack = mpCurrAnimationClip->GetTrack<IAnimationTrack>(mSelectedTrackId))
				{
					mpTrackSheetEditor->SetImGUIContext(mpImGUIContext);
					pTrack->AssignTrackForEditing(mpTrackSheetEditor.Get());
				}
			}			

			mpTrackSheetEditor->Draw(TVector2(timelineWidth, timelineHeight));
		}

		/// \note Draw a cursor
		mpImGUIContext->DrawLine(cursorPos + TVector2(currPlaybackTime * pixelsPerSecond, 0.0f), cursorPos + TVector2(currPlaybackTime * pixelsPerSecond, timelineHeight), TColorUtils::mWhite);
	}


	struct HandleTrackOperationsParams
	{
		IImGUIContext* mpImGUIContext;
		IAnimationClip* mpClip;
		TAnimationTrackId mTrackId;
		TVector2 mCursorPosition;
		F32 mTrackHeight;
		F32 mFrameWidth;
		F32 mPixelsPerSecond;

		std::function<void(TAnimationTrackId, std::string)> mOnRemoveTrackCallback = nullptr;
	};


	static bool HandleTrackOperations(const HandleTrackOperationsParams& params)
	{
		bool hasNewFrameKeyWasCreated = true;

		auto&& trackId = params.mTrackId;

		auto pImGUIContext = params.mpImGUIContext;
		auto pClip         = params.mpClip;

		const F32 frameWidth      = params.mFrameWidth;
		const F32 trackHeight     = params.mTrackHeight;
		const F32 pixelsPerSecond = params.mPixelsPerSecond;

		pImGUIContext->DisplayIDGroup(static_cast<I32>(trackId) | (1 << 24), [=, &hasNewFrameKeyWasCreated, &cursorPosition = params.mCursorPosition]
		{
			const TVector2 prevPosition = pImGUIContext->GetCursorScreenPos();
			pImGUIContext->SetCursorScreenPos(cursorPosition - TVector2(0.0f, trackHeight * 0.5f));

			pImGUIContext->Button("##TrackLine", TVector2(frameWidth, trackHeight), nullptr, true, true);

			pImGUIContext->SetCursorScreenPos(prevPosition);

			/// \note Create a new key at the position of mouse click
			if (pImGUIContext->IsItemHovered() && pImGUIContext->IsMouseDoubleClicked(0))
			{
				/// \note Convert mouse position into track's space
				if (auto pTrack = pClip->GetTrack<IAnimationTrack>(trackId))
				{
					auto newKeyHandle = pTrack->CreateKey(CMathUtils::Clamp(0.0f, frameWidth, pImGUIContext->GetMousePosition().x - cursorPosition.x) / pixelsPerSecond);
					TDE2_ASSERT(TAnimationTrackKeyId::Invalid != newKeyHandle);
				}				

				hasNewFrameKeyWasCreated = false;
			}

			/// \note Show context popup menu
			pImGUIContext->DisplayContextMenu("TracksOperationsDopeSheet", [callback = params.mOnRemoveTrackCallback, trackId](IImGUIContext& imguiContext)
			{
				imguiContext.MenuItem("Remove Track", "Del", [trackId, callback]
				{
					if (callback)
					{
						callback(trackId, Wrench::StringUtils::GetEmptyStr());
					}
				});
			});
		});

		return hasNewFrameKeyWasCreated;
	}


	static bool HandleKeySampleOperations(IImGUIContext* pImGUIContext, IAnimationClip* pClip, TAnimationTrackId trackId, TAnimationTrackKeyId keyId, 
										  const TVector2& trackOrigin, const TVector2& cursorPosition, F32 keyButtonSize, F32 frameWidth, F32 pixelsPerSecond)
	{
		bool hasSampleKeyBeenRemoved = false;

		pImGUIContext->DisplayIDGroup(static_cast<I32>(keyId) | (static_cast<I32>(trackId) << 16), [=, &hasSampleKeyBeenRemoved, &cursorPosition]
		{
			const TVector2 prevPosition = pImGUIContext->GetCursorScreenPos();
			pImGUIContext->SetCursorScreenPos(cursorPosition - TVector2(keyButtonSize * 0.5f));

			pImGUIContext->Button("##KeyButton", TVector2(keyButtonSize), nullptr, true);

			if (pImGUIContext->IsItemActive())
			{
				auto&& mousePos = pImGUIContext->GetMousePosition();

				if (pImGUIContext->IsMouseDragging(0))
				{
					if (IAnimationTrack* pTrack = pClip->GetTrack<IAnimationTrack>(trackId))
					{
						pTrack->UpdateKeyTime(keyId, CMathUtils::Clamp(0.0f, pClip->GetDuration(), (mousePos.x - trackOrigin.x) / pixelsPerSecond));
					}
				}
			}
#if 0
			if (pImGUIContext->IsMouseReleased(0))
			{
				if (IAnimationTrack* pTrack = pClip->GetTrack<IAnimationTrack>(trackId))
				{
					pTrack->UpdateKeyTime(keyId, static_cast<F32>(CMathUtils::Clamp01((mousePos.x - trackOrigin.x) / pixelsPerSecond) * pClip->GetDuration()));
				}
			}
#endif

			pImGUIContext->SetCursorScreenPos(prevPosition);

			pImGUIContext->DisplayContextMenu(Wrench::StringUtils::GetEmptyStr(), [pClip, trackId, keyId, &hasSampleKeyBeenRemoved](IImGUIContext& imguiContext)
			{
				imguiContext.MenuItem("Remove Key", "Del", [pClip, trackId, keyId, &hasSampleKeyBeenRemoved]
				{
					if (IAnimationTrack* pTrack = pClip->GetTrack<IAnimationTrack>(trackId))
					{
						E_RESULT_CODE result = pTrack->RemoveKey(keyId);
						TDE2_ASSERT(RC_OK == result);
					}

					hasSampleKeyBeenRemoved = true;
				});
			});
		});

		return !hasSampleKeyBeenRemoved;
	}


	void CAnimationEditorWindow::_drawDopesheetTracks(const TVector2& cursorPos, F32 frameWidth, F32 frameHeight, F32 pixelsPerSecond)
	{
		if (!mpCurrAnimationClip)
		{
			return;
		}

		constexpr F32 trackHeight = 17.0f;
		constexpr F32 visibleTrackHeight = 4.0f;

		const TVector2 tracksOrigin = cursorPos + TVector2(0.0f, 10.0f - mTimelineScrollPosition.y);

		F32 currVerticalPosition = 0.0f;

		const TVector2 prevCursorPosition = mpImGUIContext->GetCursorScreenPos();
		mpImGUIContext->SetCursorScreenPos(cursorPos);

		mpCurrAnimationClip->ForEachTrack([this, pixelsPerSecond, trackHeight, visibleTrackHeight, tracksOrigin, frameWidth, &currVerticalPosition](TAnimationTrackId trackId, IAnimationTrack* pTrack)
		{
			const TVector2 currPosition = tracksOrigin + TVector2(0.0f, currVerticalPosition);
			
			mpImGUIContext->DrawLine(currPosition, currPosition + TVector2(frameWidth, 0.0f), TColor32F(0.1f, 0.1f, 0.1f, 1.0f), visibleTrackHeight);
			HandleTrackOperations({ mpImGUIContext, mpCurrAnimationClip.Get(), trackId, currPosition, trackHeight, frameWidth, pixelsPerSecond });

			/// \note Draw all control points here
			auto&& samples = pTrack->GetSamples();

			for (F32 currTime : samples)
			{
				const TVector2 keyPosition = TVector2(currTime * pixelsPerSecond, 0.0f) + currPosition;

				mpImGUIContext->DrawCircle(keyPosition, 4.0f, true, TColorUtils::mWhite);
				HandleKeySampleOperations(mpImGUIContext, mpCurrAnimationClip.Get(), trackId, pTrack->GetKeyHandleByTime(currTime), currPosition, keyPosition, 16.0f, frameWidth, pixelsPerSecond);
			}

			currVerticalPosition += trackHeight;

			return true;
		});

		mpImGUIContext->SetCursorScreenPos(prevCursorPosition);
	}


	static E_RESULT_CODE CreateTrack(IAnimationClip* pClip, TypeId trackBaseTypeId, const std::string& bindingStr)
	{
		const std::unordered_map<TypeId, std::function<TAnimationTrackId()>> trackHandlers
		{
			{ GetTypeId<TVector2>::mValue, [&pClip] { return pClip->CreateTrack<CVector2AnimationTrack>(); } },
			{ GetTypeId<TVector3>::mValue, [&pClip] { return pClip->CreateTrack<CVector3AnimationTrack>(); } },
			{ GetTypeId<TQuaternion>::mValue, [&pClip] { return pClip->CreateTrack<CQuaternionAnimationTrack>(); } },
			{ GetTypeId<TColor32F>::mValue, [&pClip] { return pClip->CreateTrack<CColorAnimationTrack>(); } },
			{ GetTypeId<F32>::mValue, [&pClip] { return pClip->CreateTrack<CFloatAnimationTrack>(); } },
			{ GetTypeId<bool>::mValue, [&pClip] { return pClip->CreateTrack<CBooleanAnimationTrack>(); } },
		};

		auto it = trackHandlers.find(trackBaseTypeId);
		if (it == trackHandlers.cend())
		{
			return RC_FAIL;
		}

		const TAnimationTrackId trackHandle = (it->second)();
		if (TAnimationTrackId::Invalid == trackHandle)
		{
			return RC_FAIL;
		}

		if (auto pTrack = pClip->GetTrack<IAnimationTrack>(trackHandle))
		{
			pTrack->SetPropertyBinding(bindingStr);
		}

		return RC_OK;
	}


	void CAnimationEditorWindow::_drawPropertyBindingsWindow()
	{
		if (!mpImGUIContext->BeginModalWindow(mAddPropertyWindowId))
		{
			return;
		}

		std::function<void(CEntity*, const std::string&, bool)> processEntityTree = [this, &processEntityTree](CEntity* pEntity, const std::string& constructedBindingPath = "", bool isRoot = false)
		{
			if (!pEntity)
			{
				return;
			}

			std::string bindingPath = constructedBindingPath;

			if (std::get<0>(mpImGUIContext->BeginTreeNode(pEntity->GetName())))
			{
				if (auto pTransform = pEntity->GetComponent<CTransform>())
				{
					if (!isRoot || !pTransform->GetChildren().empty())
					{
						bindingPath.append(pEntity->GetName());
					}
				}

				/// \note Draw all the components of the entity
				for (auto pComponent : pEntity->GetComponents())
				{
					if (std::get<0>(mpImGUIContext->BeginTreeNode(pComponent->GetTypeName())))
					{
						if (!isRoot)
						{
							bindingPath.append(".");
						}

						bindingPath.append(pComponent->GetTypeName());

						for (auto currPropertyId : pComponent->GetAllProperties())
						{
							std::string currBinding = bindingPath;

							currBinding
								.append(".")
								.append(currPropertyId);

							if (mUsedPropertyBindings.find(currBinding) != mUsedPropertyBindings.cend())
							{
								continue;
							}
							
							if (mpImGUIContext->SelectableItem(currPropertyId, mCurrSelectedPropertyBinding == currBinding, false))
							{
								mCurrSelectedPropertyBinding = currBinding;

								if (auto pProperty = pComponent->GetProperty(currPropertyId))
								{
									mNewTrackTypeId = pProperty->GetValueType();
								}
							}
						}

						mpImGUIContext->EndTreeNode();
					}
				}

				/// \note Draw hierarchy
				if (auto pTransform = pEntity->GetComponent<CTransform>())
				{
					if (!pTransform->GetChildren().empty())
					{
						bindingPath.append("/");
					}

					for (TEntityId currChildEntityId : pTransform->GetChildren())
					{
						processEntityTree(mpWorld->FindEntity(currChildEntityId), bindingPath, false);
					}
				}

				mpImGUIContext->EndTreeNode();
			}
		};

		processEntityTree(mpWorld->FindEntity(mCurrAnimatedEntity), Wrench::StringUtils::GetEmptyStr(), true);

		/// Accept and Cancel buttons bar
		mpImGUIContext->BeginHorizontal();
		{
			const TVector2 buttonsSizes(mpImGUIContext->GetWindowWidth() * 0.4f, 25.0f);

			if (mpImGUIContext->Button("Accept", buttonsSizes))
			{
				if (TypeId::Invalid != mNewTrackTypeId && !mCurrSelectedPropertyBinding.empty())
				{
					/// \note Create a new track based on property's type id and property binding's value
					E_RESULT_CODE result = CreateTrack(mpCurrAnimationClip.Get(), mNewTrackTypeId, mCurrSelectedPropertyBinding);
					TDE2_ASSERT(RC_OK == result);

					mUsedPropertyBindings.insert(mCurrSelectedPropertyBinding);

					mCurrSelectedPropertyBinding = Wrench::StringUtils::GetEmptyStr();
					mpImGUIContext->CloseCurrentModalWindow();
				}
			}

			if (mpImGUIContext->Button("Cancel", buttonsSizes))
			{
				mCurrSelectedPropertyBinding = Wrench::StringUtils::GetEmptyStr();
				mpImGUIContext->CloseCurrentModalWindow();
			}
		}
		mpImGUIContext->EndHorizontal();

		mpImGUIContext->EndModalWindow();
	}


	TDE2_API IEditorWindow* CreateAnimationEditorWindow(TPtr<IResourceManager> pResourceManager, TPtr<IWorld> pWorld, TPtr<IDesktopInputContext> pInputContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CAnimationEditorWindow, result, pResourceManager, pWorld, pInputContext);
	}
}