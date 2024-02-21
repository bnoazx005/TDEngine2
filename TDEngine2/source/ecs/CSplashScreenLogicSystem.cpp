#include "../../include/ecs/CSplashScreenLogicSystem.h"
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

		mShouldSkipPredicate = params.mOnSkipAction;

		mIsInitialized = true;

		return RC_OK;
	}

	void CSplashScreenLogicSystem::InjectBindings(IWorld* pWorld)
	{
		mContext = pWorld->CreateLocalComponentsSlice<CSplashScreenItemComponent>();
	}

	void CSplashScreenLogicSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CSplashScreenLogicSystem::Update");

		const bool shouldSkipCurrScreen = mShouldSkipPredicate ? mShouldSkipPredicate() : false;
	}


	TDE2_API ISystem* CreateSplashScreenLogicSystem(const TSplashScreenModeParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CSplashScreenLogicSystem, result, params);
	}
}