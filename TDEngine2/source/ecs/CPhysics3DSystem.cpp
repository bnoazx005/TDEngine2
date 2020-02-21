#include "./../../include/ecs/CPhysics3DSystem.h"
#include "./../../include/ecs/IWorld.h"
#include "./../../include/ecs/CEntity.h"
#include "./../../include/ecs/CTransform.h"
#include "./../../include/core/IEventManager.h"
#include "./../../include/physics/3D/CBoxCollisionObject3D.h"
#include "./../../deps/bullet3/src/btBulletDynamicsCommon.h"
#include <algorithm>


namespace TDEngine2
{
	const TVector3 CPhysics3DSystem::mDefaultGravity = TVector3(0.0f, -10.0f, 0.0f);

	const F32 CPhysics3DSystem::mDefaultTimeStep = 1.0f / 60.0f;

	const U32 CPhysics3DSystem::mDefaultPositionIterations = 10;


	void CPhysics3DSystem::TPhysicsObjectsData::Clear()
	{
		mpTransforms.clear();
		mpCollisionObjects.clear();
		mpBulletColliderShapes.clear();
		mpRigidBodies.clear();
	}


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

		mCurrPositionIterations = mDefaultPositionIterations;

		mpCollisionConfiguration  = new btDefaultCollisionConfiguration();
		mpCollisionsDispatcher    = new btCollisionDispatcher(mpCollisionConfiguration);
		mpBroadphaseSolver        = new btDbvtBroadphase();
		mpImpulseConstraintSolver = new btSequentialImpulseConstraintSolver();
		mpWorld                   = new btDiscreteDynamicsWorld(mpCollisionsDispatcher, mpBroadphaseSolver, mpImpulseConstraintSolver, mpCollisionConfiguration);

		mpWorld->setGravity({ mCurrGravity.x, mCurrGravity.y, mCurrGravity.z });

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CPhysics3DSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		// \note invocation of destructors should be in reversed order of construction of these objects
		delete mpWorld;
		delete mpImpulseConstraintSolver;
		delete mpBroadphaseSolver;
		delete mpCollisionsDispatcher;
		delete mpCollisionConfiguration;

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	void CPhysics3DSystem::InjectBindings(IWorld* pWorld)
	{
		auto&& interactiveEntities = pWorld->FindEntitiesWithAny<CBoxCollisionObject3D>();

		mPhysicsObjectsData.Clear();

		CEntity* pCurrEntity = nullptr;

		CTransform* pTransform = nullptr;

		CBaseCollisionObject3D* pBaseCollisionObject = nullptr;

		btCollisionShape* pInternalColliderShape = nullptr;

		for (TEntityId currEntityId : interactiveEntities)
		{
			if (!(pCurrEntity = pWorld->FindEntity(currEntityId)))
			{
				continue;
			}

			pTransform = pCurrEntity->GetComponent<CTransform>();
			mPhysicsObjectsData.mpTransforms.push_back(pTransform);

			pBaseCollisionObject = GetValidPtrOrDefault<CBaseCollisionObject3D*>(pCurrEntity->GetComponent<CBoxCollisionObject3D>(), nullptr);
			mPhysicsObjectsData.mpCollisionObjects.push_back(pBaseCollisionObject);

			pInternalColliderShape = pBaseCollisionObject->GetCollisionShape(this);
			mPhysicsObjectsData.mpBulletColliderShapes.push_back(pInternalColliderShape);

			mPhysicsObjectsData.mpRigidBodies.push_back(_createRigidbody(*pBaseCollisionObject, pInternalColliderShape));
		}
	}

	void CPhysics3DSystem::Update(IWorld* pWorld, F32 dt)
	{
		mpWorld->stepSimulation(mCurrTimeStep, mCurrPositionIterations);

		// \note Update all transforms based on 

	}

	btBoxShape* CPhysics3DSystem::CreateBoxCollisionShape(const CBoxCollisionObject3D& box) const
	{
		TVector3 halfExtents = box.GetSizes() * 0.5f;
		return new btBoxShape({ halfExtents.x, halfExtents.y, halfExtents.z });
	}

	btRigidBody* CPhysics3DSystem::_createRigidbody(const CBaseCollisionObject3D& collisionObject, btCollisionShape* pColliderShape) const
	{
		return nullptr;
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