#include "../../include/ecs/CAnimationSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IEventManager.h"
#include "../../include/core/Meta.h"
#include "../../include/graphics/animation/CAnimationContainerComponent.h"
#include "../../include/graphics/animation/CAnimationClip.h"
#include "../../include/graphics/animation/IAnimationTrack.h"
#include "../../include/graphics/animation/AnimationTracks.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/math/MathUtils.h"
#include "../../include/editor/CPerfProfiler.h"
#include <algorithm>
#include <cmath>


namespace TDEngine2
{

	static IComponent* GetComponentByTypeName(CEntity* pEntity, const std::string& componentId)
	{
		auto&& entityComponents = pEntity->GetComponents();

		auto it = std::find_if(entityComponents.cbegin(), entityComponents.cend(), [&componentId](const IComponent* pComponent)
		{
			return pComponent->GetTypeName() == componentId;
		});

		return (it == entityComponents.cend()) ? nullptr : *it;
	}


	/// \note Binding format: Entity_1/.../Entity_N.ComponentName.property

	static IPropertyWrapperPtr ResolveBinding(IWorld* pWorld, CEntity* pEntity, const std::string& name)
	{
		TDE2_PROFILER_SCOPE("ResolveBinding, binding: " + name);

		std::string binding = Wrench::StringUtils::RemoveAllWhitespaces(name);

		std::string::size_type pos = 0;

		// \note If there are child appearances in the path go down into the hierarchy
		CEntity* pCurrEntity = pEntity;

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
				return IPropertyWrapperPtr(nullptr);
			}
		}

		// \note Check whether the component with given identifier exist or not
		const std::string& componentBinding = hierarchy.back();

		pos = componentBinding.find_first_of('.');
		if (pos == std::string::npos)
		{
			return IPropertyWrapperPtr(nullptr);
		}

		const std::string componentTypeId = componentBinding.substr(0, pos); // \note extract component's name

		if (IComponent* pSelectedComponent = GetComponentByTypeName(pCurrEntity, componentTypeId))
		{
			return pSelectedComponent->GetProperty(componentBinding.substr(pos + 1));
		}

		return IPropertyWrapperPtr(nullptr);
	}


	CAnimationSystem::CAnimationSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CAnimationSystem::Init(IResourceManager* pResourceManager, IEventManager* pEventManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager || !pEventManager)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;
		mpEventManager = pEventManager;

		mEventsHandler = CBasePropertyWrapper<std::string>::Create([this](const std::string& eventId) 
		{ 
			_notifyOnAnimationEvent(mCurrEventProviderId, eventId);
			mCurrEventProviderId = TEntityId::Invalid;

			return RC_OK; 
		}, nullptr);

		mIsInitialized = true;

		return RC_OK;
	}

	void CAnimationSystem::InjectBindings(IWorld* pWorld)
	{
		mAnimationContainersContext.mpAnimationContainers.clear();
		mAnimationContainersContext.mEntities.clear();

		for (TEntityId currEntityId : pWorld->FindEntitiesWithComponents<CAnimationContainerComponent>())
		{
			if (CEntity* pEntity = pWorld->FindEntity(currEntityId))
			{
				if (CAnimationContainerComponent* pAnimationContainer = pEntity->GetComponent<CAnimationContainerComponent>())
				{
					mAnimationContainersContext.mpAnimationContainers.push_back(pAnimationContainer);
					mAnimationContainersContext.mEntities.push_back(currEntityId);
				}
			}
		}
	}

	void CAnimationSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CAnimationSystem::Update");

		auto tryStopAnimation = [this](CAnimationContainerComponent* pAnimationContainer, TEntityId sourceId)
		{
			if (pAnimationContainer->IsStopped())
			{
				_notifyOnAnimationEvent(sourceId, TAnimationEvents::mOnFinished);
				pAnimationContainer->SetPlayingFlag(false);
			
				pAnimationContainer->GetCachedPropertiesTable().clear();

				return true;
			}

			return false;
		};

		auto& animationContainers = mAnimationContainersContext.mpAnimationContainers;
		auto& entitiesIds = mAnimationContainersContext.mEntities;

		for (USIZE i = 0; i < animationContainers.size(); ++i)
		{
			CAnimationContainerComponent* pAnimationContainer = animationContainers[i];

			const bool isStarted = pAnimationContainer->IsStarted();
			const bool isPlaying = pAnimationContainer->IsPlaying();
			const bool isPaused = pAnimationContainer->IsPaused();

			if ((!isPlaying && !isStarted) || isPaused)
			{
				tryStopAnimation(pAnimationContainer, entitiesIds[i]);
				continue;
			}

			float currTime = pAnimationContainer->GetTime() + dt;

			if (isStarted)
			{
				pAnimationContainer->SetStartedFlag(false);				
				currTime = 0.0f;

				const TResourceId animationClipId = mpResourceManager->Load<IAnimationClip>(pAnimationContainer->GetAnimationClipId());
				pAnimationContainer->SetAnimationClipResourceId(animationClipId);
				
				_notifyOnAnimationEvent(entitiesIds[i], TAnimationEvents::mOnStart);
			}

			if (tryStopAnimation(pAnimationContainer, entitiesIds[i]))
			{				
				return;
			}

			TPtr<IAnimationClip> pAnimationClip = mpResourceManager->GetResource<IAnimationClip>(pAnimationContainer->GetAnimationClipResourceId());

			const bool isLooping = pAnimationClip->GetWrapMode() == E_ANIMATION_WRAP_MODE_TYPE::LOOP;

			currTime = _adjustTimeToFitRange(currTime, isLooping, 0.0f, pAnimationClip->GetDuration());

			pAnimationContainer->SetPlayingFlag(currTime < pAnimationClip->GetDuration() ? true : isLooping);
			pAnimationContainer->SetDuration(pAnimationClip->GetDuration());
			pAnimationContainer->SetTime(currTime);

			if (!pAnimationContainer->IsPlaying())
			{
				pAnimationContainer->SetStoppedFlag(true);
			}

			auto& cachedProperties = pAnimationContainer->GetCachedPropertiesTable();

			// \note Apply values for each animation track
			pAnimationClip->ForEachTrack([pWorld, entityId = entitiesIds[i], currTime, this, pAnimationContainer, &cachedProperties](TAnimationTrackId trackId, IAnimationTrack* pTrack)
			{
				if (pTrack->GetTrackTypeId() == CEventAnimationTrack::GetTypeId()) // \note Event track's processed separately
				{
					mCurrEventProviderId = entityId;

					E_RESULT_CODE result = pTrack->Apply(mEventsHandler.Get(), currTime);
					TDE2_ASSERT(RC_OK == result);

					return true;
				}

				const U32 propertyBindingStrHash = ComputeHash(pTrack->GetPropertyBinding().c_str());

				auto&& it = cachedProperties.find(propertyBindingStrHash);
				if (it == cachedProperties.cend() || !it->second)
				{
					cachedProperties[propertyBindingStrHash] = ResolveBinding(pWorld, pWorld->FindEntity(entityId), pTrack->GetPropertyBinding());
				}

				IPropertyWrapperPtr& animableProperty = cachedProperties[propertyBindingStrHash];
				if (!animableProperty) // \note It's pretty normal case when you can't resolve binding, because, for instance, an entity may not have some component or child entity
				{
					return true;
				}

				E_RESULT_CODE result = pTrack->Apply(animableProperty.Get(), currTime); // \note apply the value to the wrapper
				TDE2_ASSERT(RC_OK == result);

				return true;
			});
		}
	}

	F32 CAnimationSystem::_adjustTimeToFitRange(F32 time, bool isLooping, F32 startTime, F32 endTime)
	{
		const F32 duration = endTime - startTime;

		if (CMathUtils::IsLessOrEqual(duration, 0.0f))
		{
			return 0.0f;
		}

		if (isLooping)
		{
			F32 t = fmodf(time - startTime, duration);  // clamp time with duration

			if (t < 0.0f)
			{
				t += duration;
			}

			t += startTime;

			return t;
		}
		
		if (CMathUtils::IsLessOrEqual(time, startTime))
		{
			return startTime;
		}

		if (CMathUtils::IsGreatOrEqual(time, endTime))
		{
			return endTime;
		}

		return time;
	}

	void CAnimationSystem::_notifyOnAnimationEvent(TEntityId id, const std::string& eventId)
	{
		TAnimationPlaybackEvent animationEvent;

		animationEvent.mAnimationSourceEntityId = id;
		animationEvent.mEventId = eventId;

		mpEventManager->Notify(&animationEvent);
	}


	TDE2_API ISystem* CreateAnimationSystem(IResourceManager* pResourceManager, IEventManager* pEventManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CAnimationSystem, result, pResourceManager, pEventManager);
	}
}