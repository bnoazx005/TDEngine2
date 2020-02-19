#include "./../../include/ecs/CPhysics3DSystem.h"
#include "./../../include/ecs/IWorld.h"
#include "./../../include/ecs/CEntity.h"
#include "./../../include/ecs/CTransform.h"
#include "./../../include/core/IEventManager.h"
#include <algorithm>


namespace TDEngine2
{
	const TVector3 CPhysics3DSystem::mDefaultGravity = TVector3(0.0f, -10.0f, 0.0f);

	const F32 CPhysics3DSystem::mDefaultTimeStep = 1.0f / 60.0f;

	const U32 CPhysics3DSystem::mDefaultVelocityIterations = 6;

	const U32 CPhysics3DSystem::mDefaultPositionIterations = 2;


	CPhysics3DSystem::CPhysics3DSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CPhysics3DSystem::Init(IEventManager* pEventManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mpEventManager = pEventManager;

		mCurrGravity = mDefaultGravity;

		mCurrTimeStep = mDefaultTimeStep;

		mCurrVelocityIterations = mDefaultVelocityIterations;
		mCurrPositionIterations = mDefaultPositionIterations;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CPhysics3DSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	void CPhysics3DSystem::InjectBindings(IWorld* pWorld)
	{
		;
	}

	void CPhysics3DSystem::Update(IWorld* pWorld, F32 dt)
	{
		;
	}


	TDE2_API ISystem* CreatePhysics3DSystem(IEventManager* pEventManager, E_RESULT_CODE& result)
	{
		CPhysics3DSystem* pSystemInstance = new (std::nothrow) CPhysics3DSystem();

		if (!pSystemInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pSystemInstance->Init(pEventManager);

		if (result != RC_OK)
		{
			delete pSystemInstance;

			pSystemInstance = nullptr;
		}

		return dynamic_cast<ISystem*>(pSystemInstance);
	}
}