#include "./../../include/ecs/CPhysics3DSystem.h"
#include "./../../include/ecs/IWorld.h"
#include "./../../include/ecs/CEntity.h"
#include "./../../include/ecs/CTransform.h"
#include "./../../include/core/IEventManager.h"
#include "./../../include/physics/3D/CBoxCollisionObject3D.h"
#include "./../../include/physics/3D/CSphereCollisionObject3D.h"
#include "./../../deps/bullet3/src/btBulletDynamicsCommon.h"
//#include "./../../deps/bullet3/src/BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
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


	CPhysics3DSystem::TEntitiesMotionState::TEntitiesMotionState(CTransform* pEntityTransform, const btTransform& startTrans, const btTransform& centerOfMassOffset):
		mpEntityTransform(pEntityTransform), mGraphicsWorldTrans(startTrans), mCenterOfMassOffset(centerOfMassOffset), mStartWorldTrans(startTrans),	mUserPointer(0)
	{
	}

	void CPhysics3DSystem::TEntitiesMotionState::getWorldTransform(btTransform & centerOfMassWorldTrans) const
	{
		centerOfMassWorldTrans = mGraphicsWorldTrans * mCenterOfMassOffset.inverse();
	}

	void CPhysics3DSystem::TEntitiesMotionState::setWorldTransform(const btTransform& centerOfMassWorldTrans)
	{
		mGraphicsWorldTrans = centerOfMassWorldTrans * mCenterOfMassOffset;

		const auto& pos = mGraphicsWorldTrans.getOrigin();
		mpEntityTransform->SetPosition({ pos.x(), pos.y(), pos.z() });

		const auto& orientation = mGraphicsWorldTrans.getRotation();
		mpEntityTransform->SetRotation(TQuaternion(orientation.x(), orientation.y(), orientation.z(), orientation.w()));
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

		E_RESULT_CODE result = _freePhysicsObjects(mPhysicsObjectsData);

		// \note invocation of destructors should be in reversed order of construction of these objects
		delete mpWorld;
		delete mpImpulseConstraintSolver;
		delete mpBroadphaseSolver;
		delete mpCollisionsDispatcher;
		delete mpCollisionConfiguration;

		mIsInitialized = false;

		delete this;

		return result;
	}

	void CPhysics3DSystem::InjectBindings(IWorld* pWorld)
	{
		auto&& interactiveEntities = pWorld->FindEntitiesWithAny<CBoxCollisionObject3D>();

		mPhysicsObjectsData.Clear();

		CEntity* pCurrEntity = nullptr;

		CTransform* pTransform = nullptr;

		CBaseCollisionObject3D* pBaseCollisionObject = nullptr;

		btCollisionShape* pInternalColliderShape = nullptr;
		btRigidBody* pCurrRigidbody = nullptr;
		btMotionState* pMotionHandler = nullptr;

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

			std::tie(pCurrRigidbody, pMotionHandler) = _createRigidbody(*pBaseCollisionObject, pTransform, pInternalColliderShape);
			mPhysicsObjectsData.mpRigidBodies.push_back(pCurrRigidbody);
			mPhysicsObjectsData.mpMotionHandlers.push_back(pMotionHandler);

			pCurrRigidbody->setUserIndex(currEntityId);

			mpWorld->addRigidBody(pCurrRigidbody);
		}
	}

	void CPhysics3DSystem::Update(IWorld* pWorld, F32 dt)
	{
		mpWorld->stepSimulation(mCurrTimeStep, mCurrPositionIterations);
	}

	btBoxShape* CPhysics3DSystem::CreateBoxCollisionShape(const CBoxCollisionObject3D& box) const
	{
		TVector3 halfExtents = box.GetSizes() * 0.5f;
		return new btBoxShape({ halfExtents.x, halfExtents.y, halfExtents.z });
	}

	btSphereShape* CPhysics3DSystem::CreateSphereCollisionShape(const CSphereCollisionObject3D& sphere) const
	{
		return new btSphereShape(sphere.GetRadius());
	}

	void CPhysics3DSystem::RaycastClosest(const TVector3& origin, const TVector3& direction, F32 maxDistance, const TOnRaycastHitCallback& onHitCallback)
	{
		TVector3 finishPos = origin + maxDistance * Normalize(direction);

		btVector3 from { origin.x, origin.y, origin.z };
		btVector3 to   { finishPos.x, finishPos.y, finishPos.z };

		btCollisionWorld::ClosestRayResultCallback closestResults(from, to);
		//closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

		mpWorld->rayTest(from, to, closestResults);

		if (closestResults.hasHit() && onHitCallback)
		{
			btVector3 point  = from.lerp(to, closestResults.m_closestHitFraction);
			btVector3 normal = closestResults.m_hitNormalWorld;

			TEntityId entityId = static_cast<U32>(closestResults.m_collisionObject->getUserIndex());

			onHitCallback({ entityId, { point.x(), point.y(), point.z() }, { normal.x(), normal.y(), normal.z() } });
		}
	}

	bool CPhysics3DSystem::RaycastAll(const TVector3& origin, const TVector3& direction, F32 maxDistance, std::vector<TRaycastResult>& hitResults)
	{
		TVector3 finishPos = origin + maxDistance * Normalize(direction);

		btVector3 from { origin.x, origin.y, origin.z };
		btVector3 to   { finishPos.x, finishPos.y, finishPos.z };

		btCollisionWorld::AllHitsRayResultCallback allResults(from, to);
		//allResults.m_flags |= btTriangleRaycastCallback::kF_KeepUnflippedNormal;
		//kF_UseGjkConvexRaytest flag is now enabled by default, use the faster but more approximate algorithm
		//allResults.m_flags |= btTriangleRaycastCallback::kF_UseSubSimplexConvexCastRaytest;
		//allResults.m_flags |= btTriangleRaycastCallback::kF_UseSubSimplexConvexCastRaytest;

		mpWorld->rayTest(from, to, allResults);

		for (int i = 0; i < allResults.m_hitFractions.size(); i++)
		{
			btVector3 point  = from.lerp(to, allResults.m_hitFractions[i]);
			btVector3 normal = allResults.m_hitNormalWorld[i];

			TEntityId entityId = static_cast<U32>(allResults.m_collisionObjects[i]->getUserIndex());

			hitResults.push_back({ entityId, { point.x(), point.y(), point.z() }, { normal.x(), normal.y(), normal.z() } });
		}

		return allResults.hasHit();
	}

	std::tuple<btRigidBody*, btMotionState*> CPhysics3DSystem::_createRigidbody(const CBaseCollisionObject3D& collisionObject, CTransform* pTransform, btCollisionShape* pColliderShape) const
	{
		E_COLLISION_OBJECT_TYPE rigidBodyType = collisionObject.GetCollisionType();

		F32 mass = collisionObject.GetMass();

		btVector3 localInertia{ 0.0f, 0.0f, 0.0f };

		if (rigidBodyType == E_COLLISION_OBJECT_TYPE::COT_DYNAMIC)
		{
			pColliderShape->calculateLocalInertia(mass, localInertia);
		}
		else
		{
			// \note Bullet3 describes static and kinematic rigid bodies as objects with zero mass
			mass = 0.0f;
		}

		btTransform internalTransform;
		{
			internalTransform.setIdentity();

			auto&& pos = pTransform->GetPosition();
			internalTransform.setOrigin({ pos.x, pos.y, pos.z });

			auto&& rot = pTransform->GetRotation();
			internalTransform.setRotation({ rot.x, rot.y, rot.z, rot.w });
		}

		btMotionState* pMotionHandler = new TEntitiesMotionState(pTransform, internalTransform);
		btRigidBody::btRigidBodyConstructionInfo rigidbodyConfiguration(mass, pMotionHandler, pColliderShape, localInertia);

		return { new btRigidBody(rigidbodyConfiguration), pMotionHandler };
	}

	E_RESULT_CODE CPhysics3DSystem::_freePhysicsObjects(TPhysicsObjectsData& physicsData)
	{
		E_RESULT_CODE result = RC_OK;

		for (auto& currMotionHandler : mPhysicsObjectsData.mpMotionHandlers)
		{
			if (!currMotionHandler)
			{
				result = result | RC_FAIL;
			}

			delete currMotionHandler;
		}

		for (auto& currRigidBody : mPhysicsObjectsData.mpRigidBodies)
		{
			if (!currRigidBody)
			{
				result = result | RC_FAIL;
			}

			mpWorld->removeRigidBody(currRigidBody);
			delete currRigidBody;
		}

		for (auto& currShape : mPhysicsObjectsData.mpBulletColliderShapes)
		{
			if (!currShape)
			{
				result = result | RC_FAIL;
			}

			delete currShape;
		}

		physicsData.Clear();

		return result;
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