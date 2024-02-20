#include "../../include/ecs/CSplashScreenLogicSystem.h"
#include "../../include/editor/CPerfProfiler.h"


namespace TDEngine2
{
	CSplashScreenLogicSystem::CSplashScreenLogicSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CSplashScreenLogicSystem::Init()
	{
		TDE2_PROFILER_SCOPE("CSplashScreenLogicSystem::Init");

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	void CSplashScreenLogicSystem::InjectBindings(IWorld* pWorld)
	{
	}

	void CSplashScreenLogicSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CSplashScreenLogicSystem::Update");
	}


	TDE2_API ISystem* CreateSplashScreenLogicSystem(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CSplashScreenLogicSystem, result);
	}
}