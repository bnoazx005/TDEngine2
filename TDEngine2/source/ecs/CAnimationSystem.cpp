#include "../../include/ecs/CAnimationSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/graphics/animation/CAnimationContainerComponent.h"
#include "../../include/graphics/animation/CAnimationClip.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/math/MathUtils.h"


namespace TDEngine2
{
	CAnimationSystem::CAnimationSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CAnimationSystem::Init(IResourceManager* pResourceManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;

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

			// \todo Update time
			// \todo Apply values for each animation track
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


	TDE2_API ISystem* CreateAnimationSystem(IResourceManager* pResourceManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CAnimationSystem, result, pResourceManager);
	}
}