#include "../include/CPhysics3DSystem.h"
#include "../include/CBoxCollisionObject3D.h"
#include "../include/CSphereCollisionObject3D.h"
#include "../include/CConvexHullCollisionObject3D.h"
#include "../include/CCapsuleCollisionObject3D.h"
#include "../include/CTrigger3D.h"
#include "../deps/bullet3/src/btBulletDynamicsCommon.h"
#include "../deps/bullet3/src/btBulletCollisionCommon.h"
#include "../deps/bullet3/src/BulletCollision/CollisionDispatch/btGhostObject.h"
#include <ecs/IWorld.h>
#include <ecs/CEntity.h>
#include <ecs/CTransform.h>
#include <core/IEventManager.h>
#include <utils/CFileLogger.h>
#include <core/CProjectSettings.h>
#include <editor/CPerfProfiler.h>
#include <algorithm>
#include <unordered_set>


namespace TDEngine2
{
	const F32 CPhysics3DSystem::mDefaultTimeStep = 1.0f / 60.0f;

	const U32 CPhysics3DSystem::mDefaultPositionIterations = 10;


	/// \fixme Replace this directive with alignas when corresponding functionality will be supported in tde2_introspector
#pragma pack(push, 16)
	struct /*alignas(16) */TEntitiesMotionState : public btMotionState
	{
		btTransform mGraphicsWorldTrans;
		btTransform mCenterOfMassOffset;
		btTransform mStartWorldTrans;

		void* mUserPointer;

		CTransform* mpEntityTransform; // \todo Replace with entity id and pointer to CWorld*

		BT_DECLARE_ALIGNED_ALLOCATOR();

		TEntitiesMotionState(CTransform* pEntityTransform, const btTransform& startTrans = btTransform::getIdentity(), const btTransform& centerOfMassOffset = btTransform::getIdentity()) :
			mpEntityTransform(pEntityTransform), mGraphicsWorldTrans(startTrans), mCenterOfMassOffset(centerOfMassOffset), mStartWorldTrans(startTrans), mUserPointer(0)
		{
		}

		void getWorldTransform(btTransform& centerOfMassWorldTrans) const
		{
			centerOfMassWorldTrans = mGraphicsWorldTrans * mCenterOfMassOffset.inverse();
		}

		void setWorldTransform(const btTransform& centerOfMassWorldTrans)
		{
			mGraphicsWorldTrans = centerOfMassWorldTrans * mCenterOfMassOffset;

			const auto& pos = mGraphicsWorldTrans.getOrigin();
			mpEntityTransform->SetPosition({ pos.x(), pos.y(), pos.z() });

			const auto& orientation = mGraphicsWorldTrans.getRotation();
			mpEntityTransform->SetRotation(TQuaternion(orientation.x(), orientation.y(), orientation.z(), orientation.w()));
		}
	};

#pragma pack(pop)



	void CPhysics3DSystem::TPhysicsObjectsData::Clear()
	{
		mpTransforms.clear();
		mpCollisionObjects.clear();
		mpBulletColliderShapes.clear();
		mpInternalCollisionObjects.clear();
		mpTriggers.clear();
		mInUseTable.clear();
	}

	void CPhysics3DSystem::TPhysicsObjectsData::EraseItem(USIZE index)
	{
		TDE2_ASSERT(mpTransforms.size() > index);

		mpTransforms.erase(mpTransforms.begin() + index);
		mpCollisionObjects.erase(mpCollisionObjects.begin() + index);
		mpBulletColliderShapes.erase(mpBulletColliderShapes.begin() + index);
		mpInternalCollisionObjects.erase(mpInternalCollisionObjects.begin() + index);
		mpTriggers.erase(mpTriggers.begin() + index);
		mInUseTable.erase(mInUseTable.begin() + index);
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

		mCurrGravity = CProjectSettings::Get()->m3DPhysicsSettings.mGravity;

		mCurrTimeStep = mDefaultTimeStep;

		mCurrPositionIterations = mDefaultPositionIterations;

		mpCollisionConfiguration  = new btDefaultCollisionConfiguration();
		mpCollisionsDispatcher    = new btCollisionDispatcher(mpCollisionConfiguration);
		mpBroadphaseSolver        = new btDbvtBroadphase();
		mpImpulseConstraintSolver = new btSequentialImpulseConstraintSolver();
		mpWorld                   = new btDiscreteDynamicsWorld(mpCollisionsDispatcher, mpBroadphaseSolver, mpImpulseConstraintSolver, mpCollisionConfiguration);
		
		mpBroadphaseSolver->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());

		mpWorld->setGravity({ mCurrGravity.x, mCurrGravity.y, mCurrGravity.z });

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CPhysics3DSystem::_onFreeInternal()
	{
		E_RESULT_CODE result = _freePhysicsObjects(mPhysicsObjectsData);

		// \note invocation of destructors should be in reversed order of construction of these objects
		delete mpWorld;
		delete mpImpulseConstraintSolver;
		delete mpBroadphaseSolver;
		delete mpCollisionsDispatcher;
		delete mpCollisionConfiguration;

		return result;
	}

	void CPhysics3DSystem::InjectBindings(IWorld* pWorld)
	{
		auto& transforms = mPhysicsObjectsData.mpTransforms;
		auto& collisionObjects = mPhysicsObjectsData.mpCollisionObjects;
		auto& collisionShapes = mPhysicsObjectsData.mpBulletColliderShapes;
		auto& usageTable = mPhysicsObjectsData.mInUseTable;

		CBaseCollisionObject3D* pBaseCollisionObject = nullptr;
		CEntity* pCurrEntity = nullptr;
		CTransform* pTransform = nullptr;

		btCollisionShape* pInternalColliderShape = nullptr;
		btCollisionObject* pCurrCollisionObject = nullptr;
		btMotionState* pMotionHandler = nullptr;

		/// \note Reset usage flags
		usageTable.resize(transforms.size());

		for (USIZE i = 0; i < usageTable.size(); i++)
		{
			usageTable[i] = false;
		}

		/// \note If there is no given entity in the context add it
		for (const TEntityId currEntityId : pWorld->FindEntitiesWithAny<CBoxCollisionObject3D, CSphereCollisionObject3D, CConvexHullCollisionObject3D>())
		{
			auto it = std::find_if(transforms.begin(), transforms.end(), [currEntityId](auto&& element) { return element->GetOwnerId() == currEntityId; });
			if (it != transforms.end())
			{
				/// \note The entity's already registered, but parameters could be changed
				usageTable[static_cast<USIZE>(std::distance(transforms.begin(), it))] = true;

				/// \todo Retrieve collision object and its 'dirty' flag
				/// \todo If the flag is true that means we should recreate physics representation for the entity

				continue;
			}

			if (!(pCurrEntity = pWorld->FindEntity(currEntityId)))
			{
				continue;
			}

			pBaseCollisionObject = GetValidPtrOrDefault<CBaseCollisionObject3D*>(pCurrEntity->GetComponent<CBoxCollisionObject3D>(),
				GetValidPtrOrDefault<CBaseCollisionObject3D*>(pCurrEntity->GetComponent<CSphereCollisionObject3D>(),
					pCurrEntity->GetComponent<CConvexHullCollisionObject3D>()));

			collisionObjects.push_back(pBaseCollisionObject);

			pInternalColliderShape = pBaseCollisionObject->GetCollisionShape(this);
			collisionShapes.push_back(pInternalColliderShape);

			pTransform = pCurrEntity->GetComponent<CTransform>();
			transforms.push_back(pTransform);

			if (pCurrEntity->HasComponent<CTrigger3D>())
			{
				btPairCachingGhostObject* pPairGhostObject = nullptr;

				std::tie(pPairGhostObject, pMotionHandler) = _createTrigger(*pBaseCollisionObject, pTransform, pInternalColliderShape);
				mPhysicsObjectsData.mpTriggers.push_back(pPairGhostObject);

				mpWorld->addCollisionObject(pPairGhostObject, btBroadphaseProxy::SensorTrigger, btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger);
				pCurrCollisionObject = btPairCachingGhostObject::upcast(pPairGhostObject);
			}
			else
			{
				std::tie(pCurrCollisionObject, pMotionHandler) = _createRigidbody(*pBaseCollisionObject, pTransform, pInternalColliderShape);
				mpWorld->addRigidBody(btRigidBody::upcast(pCurrCollisionObject));

				mPhysicsObjectsData.mpTriggers.push_back(nullptr);
			}

			pCurrCollisionObject->setUserIndex(static_cast<U32>(currEntityId));

			mPhysicsObjectsData.mpInternalCollisionObjects.push_back(pCurrCollisionObject);
			mPhysicsObjectsData.mpMotionHandlers.push_back(pMotionHandler);
			usageTable.push_back(true);
		}

		/// \note Remove unused entities from the internal world
		auto it = usageTable.begin();

		while (it != usageTable.end())
		{
			if (*it)
			{
				it++;
				continue;
			}

			const USIZE index = static_cast<USIZE>(std::distance(usageTable.begin(), it));
			mPhysicsObjectsData.EraseItem(index);
			
			it = usageTable.begin() + index;
		}
	}


	struct TCollisionObjectTestCallback : btCollisionWorld::ContactResultCallback
	{
		public:
			explicit TCollisionObjectTestCallback():
				mCollidedPairs()
			{
				static_assert(2 * sizeof(U32) >= sizeof(U64), "U64 type should be at least two times bigger than U32");
			}

			void PreCollisionTests()
			{
				mPrevCollidedPairsHashTable.clear();
				std::copy(mCurrCollidedPairsHashTable.begin(), mCurrCollidedPairsHashTable.end(), std::inserter(mPrevCollidedPairsHashTable, mPrevCollidedPairsHashTable.begin()));

				mCurrCollidedPairsHashTable.clear();
				mCollidedPairs.clear();
			}

			btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) override
			{
				const U32 firstColliderId = static_cast<U32>(colObj0Wrap->getCollisionObject()->getUserIndex());
				const U32 secondColliderId = static_cast<U32>(colObj1Wrap->getCollisionObject()->getUserIndex());

				auto it = std::find_if(mCollidedPairs.begin(), mCollidedPairs.end(), [firstColliderId, secondColliderId](auto&& entity)
				{
					return (std::get<0>(entity) == firstColliderId && std::get<1>(entity) == secondColliderId) ||
						(std::get<1>(entity) == firstColliderId && std::get<0>(entity) == secondColliderId);
				});

				/// \note Don't add duplicates
				if (it == mCollidedPairs.end())
				{
					const btVector3& normal = cp.m_normalWorldOnB;
					mCollidedPairs.emplace_back(std::make_tuple(firstColliderId, secondColliderId, Normalize(TVector3(normal.x(), normal.y(), normal.z()))));
					
					mCurrCollidedPairsHashTable.insert(static_cast<U64>(static_cast<U64>(firstColliderId) << sizeof(U32) | secondColliderId));
				}

				return 0.0f;
			}

			void NotifyPhysicsEvents(IEventManager* pEventManager)
			{
				U32 firstHandle = 0;
				U32 secondHandle = 0;
				TVector3 normal;

				for (auto&& currPair : mCollidedPairs)
				{
					std::tie(firstHandle, secondHandle, normal) = currPair;

					TOn3DCollisionRegisteredEvent collisionEventData;
					collisionEventData.mEntities[0] = static_cast<TEntityId>(firstHandle);
					collisionEventData.mEntities[1] = static_cast<TEntityId>(secondHandle);
					collisionEventData.mContactNormal = normal;

					const U64 hash = static_cast<U64>(static_cast<U64>(firstHandle) << sizeof(U32) | secondHandle);

					collisionEventData.mType = mPrevCollidedPairsHashTable.find(hash) != mPrevCollidedPairsHashTable.end() ?
						TOn3DCollisionRegisteredEvent::E_COLLISION_EVENT_TYPE::ON_STAY :
						TOn3DCollisionRegisteredEvent::E_COLLISION_EVENT_TYPE::ON_ENTER;

					pEventManager->Notify(&collisionEventData);
				}

				/// \note Check up exit events
				if (!mPrevCollidedPairsHashTable.empty())
				{
					static std::vector<U64> diff;
					diff.clear();

					std::copy_if(mCurrCollidedPairsHashTable.begin(), mCurrCollidedPairsHashTable.end(), std::back_inserter(diff), [this](auto&& entity)
					{
						return mPrevCollidedPairsHashTable.find(entity) == mPrevCollidedPairsHashTable.end();
					});

					for (U64 currHash : diff)
					{
						TOn3DCollisionRegisteredEvent collisionEventData;
						collisionEventData.mEntities[0] = static_cast<TEntityId>(currHash >> sizeof(U32));
						collisionEventData.mEntities[1] = static_cast<TEntityId>(currHash & (std::numeric_limits<U32>::max)());
						collisionEventData.mType = TOn3DCollisionRegisteredEvent::E_COLLISION_EVENT_TYPE::ON_EXIT;

						pEventManager->Notify(&collisionEventData);
					}
				}
			}
		private:
			std::vector<std::tuple<U32, U32, TVector3>> mCollidedPairs;
			std::unordered_set<U64>                     mPrevCollidedPairsHashTable;
			std::unordered_set<U64>                     mCurrCollidedPairsHashTable;
	};


	static void UpdateKinematicObjects(std::vector<CTransform*>& transforms, std::vector<btCollisionObject*>& collisionObjects, btDiscreteDynamicsWorld* pPhysicsWorld, IEventManager* pEventManager)
	{
		TDE2_PROFILER_SCOPE("CPhysics3DSystem::UpdateKinematicObjects");
		static TCollisionObjectTestCallback callback;
		
		callback.PreCollisionTests();
		
		for (USIZE i = 0; i < collisionObjects.size(); ++i)
		{
			auto& pCollisionObject = collisionObjects[i];

			if (!pCollisionObject->isStaticOrKinematicObject())
			{
				continue;
			}

			CTransform* pTransform = transforms[i];

			const TVector3 position = pTransform->GetPosition();
			const TQuaternion rotation = pTransform->GetRotation();

			btTransform newTransform;
			newTransform.setIdentity();

			newTransform.setOrigin(btVector3(position.x, position.y, position.z));
			newTransform.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));

			pCollisionObject->setWorldTransform(newTransform);

			pPhysicsWorld->contactTest(pCollisionObject, callback);
		}

		callback.NotifyPhysicsEvents(pEventManager);
	}


	void CPhysics3DSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CPhysics3DSystem::Update");

		// \fixme This is bad solution, it's used to force activate all rigidbodies for smooth update
		auto rigidbodies = mpWorld->getNonStaticRigidBodies();
		for (I32 i = 0; i < rigidbodies.size(); ++i)
		{
			rigidbodies[i]->activate(true);
		}
		
		mpWorld->stepSimulation(mCurrTimeStep, mCurrPositionIterations);

		UpdateKinematicObjects(mPhysicsObjectsData.mpTransforms, mPhysicsObjectsData.mpInternalCollisionObjects, mpWorld, mpEventManager);

#if 1
		const btCollisionObject* pColliderObject = nullptr;

		for (auto&& pCurrTrigger : mPhysicsObjectsData.mpTriggers)
		{
			if (!pCurrTrigger)
			{
				continue;
			}

			btManifoldArray manifoldArray;
			auto&& overlappingPairs = pCurrTrigger->getOverlappingPairCache()->getOverlappingPairArray();

			//mpWorld->getDispatcher()->dispatchAllCollisionPairs(pCurrTrigger->getOverlappingPairCache(), mpWorld->getDispatchInfo(), mpWorld->getDispatcher());

			for (I32 i = 0; i < overlappingPairs.size(); ++i)
			{
				manifoldArray.clear();

				btBroadphasePair* pCollisionPair = mpWorld->getPairCache()->findPair(overlappingPairs[i].m_pProxy0, overlappingPairs[i].m_pProxy1);
				
				if (!pCollisionPair)
				{
					continue;
				}

				if (pCollisionPair->m_algorithm)
				{
					pCollisionPair->m_algorithm->getAllContactManifolds(manifoldArray);
				}

				for (int j = 0; j < manifoldArray.size(); j++)
				{
					for (int p = 0; p < manifoldArray[j]->getNumContacts(); p++)
					{
						const btManifoldPoint& point = manifoldArray[j]->getContactPoint(p);

						if (point.getDistance() < 0.0f)
						{
							TOnTrigger3DEvent trigger3DEventData;
							trigger3DEventData.mEntities[0] = static_cast<TEntityId>(pCurrTrigger->getUserIndex());
							trigger3DEventData.mEntities[1] = static_cast<TEntityId>(((pCurrTrigger == manifoldArray[j]->getBody0()) ? manifoldArray[j]->getBody1() : manifoldArray[j]->getBody0())->getUserIndex());

							mpEventManager->Notify(&trigger3DEventData);

							continue;
						}
					}
				}
			}

		}
#endif
	}

#if TDE2_EDITORS_ENABLED
	
	void CPhysics3DSystem::DebugOutput(IDebugUtility* pDebugUtility, F32 dt) const
	{
		for (auto&& pCurrCollisionObject : mPhysicsObjectsData.mpInternalCollisionObjects)
		{
		}
	}

#endif

	btBoxShape* CPhysics3DSystem::CreateBoxCollisionShape(const CBoxCollisionObject3D& box) const
	{
		TVector3 halfExtents = box.GetSizes() * 0.5f;
		return new btBoxShape({ halfExtents.x, halfExtents.y, halfExtents.z });
	}

	btSphereShape* CPhysics3DSystem::CreateSphereCollisionShape(const CSphereCollisionObject3D& sphere) const
	{
		return new btSphereShape(sphere.GetRadius());
	}

	btConvexHullShape* CPhysics3DSystem::CreateConvexHullCollisionShape(const CConvexHullCollisionObject3D& hull) const
	{
		auto pHullShape = new btConvexHullShape();

		for (auto&& currVertex : hull.GetVertices())
		{
			pHullShape->addPoint(btVector3(currVertex.x, currVertex.y, currVertex.z));
		}

		return pHullShape;
	}

	btCapsuleShape* CPhysics3DSystem::CreateCapsuleCollisionShape(const CCapsuleCollisionObject3D& capsule) const
	{
		return new btCapsuleShape(capsule.GetRadius(), capsule.GetHeight());
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

			TEntityId entityId = static_cast<TEntityId>(closestResults.m_collisionObject->getUserIndex());

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

			TEntityId entityId = static_cast<TEntityId>(allResults.m_collisionObjects[i]->getUserIndex());

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

		btRigidBody* pRigidBody = new btRigidBody(rigidbodyConfiguration);

		if (E_COLLISION_OBJECT_TYPE::COT_KINEMATIC == rigidBodyType)
		{
			pRigidBody->setCollisionFlags(pRigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
			pRigidBody->setActivationState(DISABLE_DEACTIVATION);
		}

		return { pRigidBody, pMotionHandler };
	}

	std::tuple<btPairCachingGhostObject*, btMotionState*> CPhysics3DSystem::_createTrigger(const CBaseCollisionObject3D& collisionObject, CTransform* pTransform, btCollisionShape* pColliderShape) const
	{
		E_COLLISION_OBJECT_TYPE triggerType = collisionObject.GetCollisionType();

		F32 mass = collisionObject.GetMass();

		btVector3 localInertia{ 0.0f, 0.0f, 0.0f };

		if (triggerType == E_COLLISION_OBJECT_TYPE::COT_DYNAMIC)
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
		
		auto pTriggerObject = new btPairCachingGhostObject();

		pTriggerObject->setCollisionShape(pColliderShape);
		pTriggerObject->setCollisionFlags(pTriggerObject->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);

		switch (triggerType)
		{
			case E_COLLISION_OBJECT_TYPE::COT_KINEMATIC:
				pTriggerObject->setCollisionFlags(pTriggerObject->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
				break;
			case E_COLLISION_OBJECT_TYPE::COT_STATIC:
				pTriggerObject->setCollisionFlags(pTriggerObject->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
				break;
		}

		if (E_COLLISION_OBJECT_TYPE::COT_DYNAMIC != triggerType)
		{
			pTriggerObject->setActivationState(DISABLE_DEACTIVATION);
		}

		return { pTriggerObject, pMotionHandler };
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

		for (auto& pCurrObject : mPhysicsObjectsData.mpInternalCollisionObjects)
		{
			if (!pCurrObject)
			{
				result = result | RC_FAIL;
			}

			mpWorld->removeCollisionObject(pCurrObject);
			delete pCurrObject;
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
		return CREATE_IMPL(ISystem, CPhysics3DSystem, result, pEventManager);
	}
}