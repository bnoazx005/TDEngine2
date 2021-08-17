#include "../include/CAnimationEditorWindow.h"


namespace TDEngine2
{
	const std::string CAnimationEditorWindow::mAddPropertyWindowId = "Add Property ...";


	CAnimationEditorWindow::CAnimationEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CAnimationEditorWindow::Init(IResourceManager* pResourceManager, IWorld* pWorld)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pResourceManager || !pWorld)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;
		mpWorld = pWorld;

		mCurrAnimatedEntity = TEntityId::Invalid;

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


	void CAnimationEditorWindow::_drawTracksHierarchy(F32 blockWidth)
	{
		mpImGUIContext->SetCursorScreenPos(mpImGUIContext->GetCursorScreenPos() + TVector2(0.0f, 25.0f));

		mpImGUIContext->BeginChildWindow("##TracksHierarchyWidget", TVector2(blockWidth - 10.0f, mpImGUIContext->GetWindowHeight() * 0.65f));
		{
			mpCurrAnimationClip->ForEachTrack([this](TAnimationTrackId trackId, IAnimationTrack* pTrack)
			{
				const std::string& propertyBinding = pTrack->GetPropertyBinding();

				std::string entityName, bindingName;
				std::tie(entityName, bindingName) = GetTrackInfoFromBinding(mpWorld, mCurrAnimatedEntity, propertyBinding);

				if (mUsedPropertyBindings.find(propertyBinding) == mUsedPropertyBindings.cend())
				{
					mUsedPropertyBindings.insert(propertyBinding);
				}

				/// \todo For events track there should be a unique identifier
				if (mpImGUIContext->SelectableItem(Wrench::StringUtils::Format("{0}: {1}", entityName, bindingName), mSelectedTrackId == trackId))
				{					
					mSelectedTrackId = trackId;
				}

				/// \todo Remove the selected track
				if (mSelectedTrackId == trackId)
				{
					mpImGUIContext->DisplayContextMenu("TracksOperations", [this, trackId, propertyBinding](IImGUIContext& imguiContext)
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
					});

					if (TAnimationTrackId::Invalid == mSelectedTrackId)
					{
						return false;
					}
				}

				return true;
			});
		}
		mpImGUIContext->EndChildWindow();

		mpImGUIContext->BeginChildWindow("##TracksHierarchyToolbarWidget", TVector2(blockWidth - 10.0f, 35.0f));
		{
			_drawPropertyBindingsWindow();

			if (mpImGUIContext->Button("Add Property", TVector2(mpImGUIContext->GetWindowWidth() * 0.5f, 25.0f)))
			{
				mpImGUIContext->ShowModalWindow(mAddPropertyWindowId);
			}
		}
		mpImGUIContext->EndChildWindow();
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

			if (mIsDopeSheetModeEnabled)
			{
				_drawDopesheetWidget(playbackTime);
			}
			else
			{
				const F32 timelineWidth = mpImGUIContext->GetWindowWidth();
				const F32 timelineHeight = mpImGUIContext->GetWindowHeight() * 0.95f;

				CAnimationCurveEditorWindow::DrawCurveEditor(mpImGUIContext, timelineWidth, timelineHeight, nullptr);
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
				/// \note Create a new track based on property's type id and property binding's value
				E_RESULT_CODE result = CreateTrack(mpCurrAnimationClip, mNewTrackTypeId, mCurrSelectedPropertyBinding);
				TDE2_ASSERT(RC_OK == result);

				mUsedPropertyBindings.insert(mCurrSelectedPropertyBinding);

				mCurrSelectedPropertyBinding = Wrench::StringUtils::GetEmptyStr();
				mpImGUIContext->CloseCurrentModalWindow();
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


	TDE2_API IEditorWindow* CreateAnimationEditorWindow(IResourceManager* pResourceManager, IWorld* pWorld, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CAnimationEditorWindow, result, pResourceManager, pWorld);
	}
}