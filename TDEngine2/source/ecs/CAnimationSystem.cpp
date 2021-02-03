#include "../../include/ecs/CAnimationSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IEventManager.h"
#include "../../include/graphics/animation/CAnimationContainerComponent.h"
#include "../../include/graphics/animation/CAnimationClip.h"
#include "../../include/graphics/animation/IAnimationTrack.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/math/MathUtils.h"


namespace TDEngine2
{
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

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CAnimationSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	void CAnimationSystem::InjectBindings(IWorld* pWorld)
	{
		mAnimatedEntities = pWorld->FindEntitiesWithComponents<CAnimationContainerComponent>();
	}

	void CAnimationSystem::Update(IWorld* pWorld, F32 dt)
	{
		CEntity* pCurrEntity = nullptr;

		for (TEntityId currEntityId : mAnimatedEntities)
		{
			pCurrEntity = pWorld->FindEntity(currEntityId);
			if (!pCurrEntity)
			{
				continue;
			}

			CAnimationContainerComponent* pAnimationContainer = pCurrEntity->GetComponent<CAnimationContainerComponent>();

			const bool isStarted = pAnimationContainer->IsStarted();
			const bool isPlaying = pAnimationContainer->IsPlaying();
			const bool isPaused = pAnimationContainer->IsPaused();

			if ((!isPlaying && !isStarted) || isPaused)
			{
				continue;
			}

			float currTime = pAnimationContainer->GetTime() + dt;

			if (isStarted)
			{
				pAnimationContainer->SetStartedFlag(false);				
				currTime = 0.0f;

				const TResourceId animationClipId = mpResourceManager->Load<CAnimationClip>(pAnimationContainer->GetAnimationClipId());
				pAnimationContainer->SetAnimationClipResourceId(animationClipId);
				
				_notifyOnAnimationEvent(pCurrEntity->GetId(), TAnimationEvents::mOnStart);
			}

			if (pAnimationContainer->IsStopped())
			{
				_notifyOnAnimationEvent(pCurrEntity->GetId(), TAnimationEvents::mOnFinished);
				pAnimationContainer->SetPlayingFlag(false);

				return;
			}

			IAnimationClip* pAnimationClip = mpResourceManager->GetResource<IAnimationClip>(pAnimationContainer->GetAnimationClipResourceId());

			const bool isLooping = pAnimationClip->GetWrapMode() == E_ANIMATION_WRAP_MODE_TYPE::LOOP;

			currTime = _adjustTimeToFitRange(currTime, isLooping, 0.0f, pAnimationClip->GetDuration());

			pAnimationContainer->SetPlayingFlag(currTime < pAnimationClip->GetDuration() ? true : isLooping);
			pAnimationContainer->SetDuration(pAnimationClip->GetDuration());
			pAnimationContainer->SetTime(currTime);

			if (!pAnimationContainer->IsPlaying())
			{
				pAnimationContainer->SetStoppedFlag(true);
			}

			// \note Apply values for each animation track
			pAnimationClip->ForEachTrack([pCurrEntity](IAnimationTrack* pTrack)
			{
				// resolve binding 
				// if corresponding child entity or a component exist get their property wrapper
				// apply the value to the wrapper
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
			F32 t = std::fmodf(time - startTime, duration);  // clamp time with duration

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