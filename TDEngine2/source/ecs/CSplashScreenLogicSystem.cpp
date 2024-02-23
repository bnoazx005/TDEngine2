#include "../../include/ecs/CSplashScreenLogicSystem.h"
#include "../../include/ecs/CWorld.h"
#include "../../include/core/IEventManager.h"
#include "../../include/scene/components/CSplashScreenItemComponent.h"
#include "../../include/editor/CPerfProfiler.h"


namespace TDEngine2
{
	CSplashScreenLogicSystem::CSplashScreenLogicSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CSplashScreenLogicSystem::Init(const TSplashScreenModeParams& params)
	{
		TDE2_PROFILER_SCOPE("CSplashScreenLogicSystem::Init");

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!params.mpEventManager)
		{
			return RC_INVALID_ARGS;
		}

		mShouldSkipPredicate = params.mOnSkipAction;
		mpEventManager = params.mpEventManager;

		mIsInitialized = true;

		return RC_OK;
	}

	void CSplashScreenLogicSystem::InjectBindings(IWorld* pWorld)
	{
		if (!mIsFirstTimeInvokation)
		{
			return;
		}

		auto&& entities = pWorld->FindEntitiesWithComponents<CSplashScreenContainerRoot>();
		if (entities.empty())
		{
			return;
		}

		CEntity* pContainerRootEntity = pWorld->FindEntity(entities.front());
		if (!pContainerRootEntity)
		{
			return;
		}

		CTransform* pContainerRootTransform = pContainerRootEntity->GetComponent<CTransform>();
		auto&& childrenEntities = pContainerRootTransform->GetChildren();

		AddDefferedCommand([pWorld, &childrenEntities]
		{
			for (USIZE i = 0; i < childrenEntities.size(); i++)
			{
				SetEntityActive(pWorld, childrenEntities[i], i == 0);
			}
		});

		mIsFirstTimeInvokation = false;

	}

	void CSplashScreenLogicSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CSplashScreenLogicSystem::Update");
		
		auto&& entities = pWorld->FindEntitiesWithComponents<CSplashScreenContainerRoot>();
		if (entities.empty())
		{
			return;
		}

		CEntity* pContainerRootEntity = pWorld->FindEntity(entities.front());
		if (!pContainerRootEntity)
		{
			return;
		}

		CTransform* pContainerRootTransform = pContainerRootEntity->GetComponent<CTransform>();
		auto&& childrenEntities = pContainerRootTransform->GetChildren();

		if (mCurrActiveScreenIndex >= childrenEntities.size())
		{
			AddDefferedCommand([this]
			{
				TOnSplashScreensFinishedEvent spashScreenFinishedEvent;
				mpEventManager->Notify(&spashScreenFinishedEvent);
			});

			return;
		}

		CEntity* pCurrSplashScreenEntity = pWorld->FindEntity(childrenEntities[mCurrActiveScreenIndex]);
		if (!pCurrSplashScreenEntity)
		{
			return;
		}

		CSplashScreenItemComponent* pSplashScreenItem = pCurrSplashScreenEntity->GetComponent<CSplashScreenItemComponent>();
		if (!pSplashScreenItem)
		{
			mCurrActiveScreenIndex++;
			return;
		}

		const bool shouldSkipCurrScreen = mShouldSkipPredicate ? mShouldSkipPredicate() : false;

		if (shouldSkipCurrScreen || pSplashScreenItem->mElapsedTime > pSplashScreenItem->mDuration)
		{
			AddDefferedCommand([this, pWorld, entityId = pCurrSplashScreenEntity->GetId(), &childrenEntities]
			{
				SetEntityActive(pWorld, entityId, false); // \todo Replace with animation events
			
				mCurrActiveScreenIndex++;

				if (mCurrActiveScreenIndex >= childrenEntities.size())
				{
					return;
				}

				SetEntityActive(pWorld, childrenEntities[mCurrActiveScreenIndex], true); // \todo Replace with animation events
			});

			return;
		}

		pSplashScreenItem->mElapsedTime += dt;		
	}


	TDE2_API ISystem* CreateSplashScreenLogicSystem(const TSplashScreenModeParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CSplashScreenLogicSystem, result, params);
	}
}