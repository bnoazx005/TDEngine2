#include "./../../include/ecs/CPhysics2DSystem.h"
#include "./../../include/ecs/IWorld.h"
#include "./../../include/ecs/CEntity.h"
#include "./../../include/ecs/CTransform.h"


namespace TDEngine2
{
	const TVector2 CPhysics2DSystem::mDefaultGravity = TVector2(0.0f, -10.0f);

	const F32 CPhysics2DSystem::mDefaultTimeStep = 1.0f / 60.0f;

	const U32 CPhysics2DSystem::mDefaultVelocityIterations = 6;

	const U32 CPhysics2DSystem::mDefaultPositionIterations = 2;


	CPhysics2DSystem::CPhysics2DSystem() :
		CBaseObject(), mpWorldInstance(nullptr)
	{
	}

	E_RESULT_CODE CPhysics2DSystem::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mpWorldInstance = new b2World({ mDefaultGravity.x, mDefaultGravity.y });

		mCurrGravity = mDefaultGravity;

		mCurrTimeStep = mDefaultTimeStep;

		mCurrVelocityIterations = mDefaultVelocityIterations;

		mCurrPositionIterations = mDefaultPositionIterations;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CPhysics2DSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (mpWorldInstance)
		{
			delete mpWorldInstance;

			mpWorldInstance = nullptr;
		}
		
		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	void CPhysics2DSystem::InjectBindings(IWorld* pWorld)
	{
		std::vector<TEntityId> entities = pWorld->FindEntitiesWithComponents<CTransform>();

		mTransforms.clear();

		CEntity* pCurrEntity = nullptr;

		for (auto iter = entities.begin(); iter != entities.end(); ++iter)
		{
			pCurrEntity = pWorld->FindEntity(*iter);

			if (!pCurrEntity)
			{
				continue;
			}

			mTransforms.push_back(pCurrEntity->GetComponent<CTransform>());

			//mSprites.push_back(pCurrEntity->GetComponent<CQuadSprite>());
		}
	}

	void CPhysics2DSystem::Update(IWorld* pWorld, F32 dt)
	{
		mpWorldInstance->Step(mCurrTimeStep, mCurrVelocityIterations, mCurrPositionIterations);
	}
	

	TDE2_API ISystem* CreatePhysics2DSystem(E_RESULT_CODE& result)
	{
		CPhysics2DSystem* pSystemInstance = new (std::nothrow) CPhysics2DSystem();

		if (!pSystemInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pSystemInstance->Init();

		if (result != RC_OK)
		{
			delete pSystemInstance;

			pSystemInstance = nullptr;
		}

		return dynamic_cast<ISystem*>(pSystemInstance);
	}
}