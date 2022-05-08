#include "../../include/ecs/CPhysics2DSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/physics/2D/CBoxCollisionObject2D.h"
#include "../../include/physics/2D/CCircleCollisionObject2D.h"
#include "../../include/physics/2D/CTrigger2D.h"
#include "../../include/core/IEventManager.h"
#include "../../include/editor/CPerfProfiler.h"
#include <algorithm>


namespace TDEngine2
{
	const TVector2 CPhysics2DSystem::mDefaultGravity = TVector2(0.0f, -10.0f);

	const F32 CPhysics2DSystem::mDefaultTimeStep = 1.0f / 60.0f;

	const U32 CPhysics2DSystem::mDefaultVelocityIterations = 6;

	const U32 CPhysics2DSystem::mDefaultPositionIterations = 2;


	CPhysics2DSystem::CPhysics2DSystem() :
		CBaseSystem(), mpWorldInstance(nullptr)
	{
	}

	E_RESULT_CODE CPhysics2DSystem::Init(IEventManager* pEventManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mpWorldInstance = new b2World({ mDefaultGravity.x, mDefaultGravity.y });
		mpWorldInstance->SetContactListener(mpContactsListener = new CTriggerContactsListener(pEventManager, mCollidersData.mBodies, mHandles2EntitiesMap));

		mpEventManager = pEventManager;

		mCurrGravity = mDefaultGravity;

		mCurrTimeStep = mDefaultTimeStep;

		mCurrVelocityIterations = mDefaultVelocityIterations;
		mCurrPositionIterations = mDefaultPositionIterations;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CPhysics2DSystem::_onFreeInternal()
	{
		if (mpWorldInstance)
		{
			delete mpWorldInstance;

			mpWorldInstance = nullptr;
		}
		
		delete mpContactsListener;

		return RC_OK;
	}

	void CPhysics2DSystem::InjectBindings(IWorld* pWorld)
	{
		auto&& interactiveEntities = pWorld->FindEntitiesWithAny<CBoxCollisionObject2D, CCircleCollisionObject2D, CTrigger2D>();
		
		/// Remove all bodies from the world's instance
		for (auto pCurrBody : mCollidersData.mBodies)
		{
			if (!pCurrBody)
			{
				continue;
			}

			mpWorldInstance->DestroyBody(pCurrBody);
		}

		mHandles2EntitiesMap.clear();
		mCollidersData.Clear();

		CEntity* pCurrEntity = nullptr;

		CTransform* pTransform = nullptr;

		CBaseCollisionObject2D* pCollisionObject = nullptr;

		b2Body* pCurrBody = nullptr;

		CTrigger2D* pCurrTrigger = nullptr;

		for (TEntityId currEntityId : interactiveEntities)
		{
			if (!(pCurrEntity = pWorld->FindEntity(currEntityId)))
			{
				continue;
			}

			mHandles2EntitiesMap[static_cast<U32>(mCollidersData.mTransforms.size())] = currEntityId;

			pTransform = pCurrEntity->GetComponent<CTransform>();
			
			mCollidersData.mTransforms.push_back(pTransform);
			mCollidersData.mCollisionObjects.push_back(pCollisionObject);

			if (pCurrTrigger = pCurrEntity->GetComponent<CTrigger2D>())
			{
				mCollidersData.mTriggers.push_back(pCurrTrigger);
			}

			pCurrBody = _createPhysicsBody(pTransform, pCurrTrigger,
										   GetValidPtrOrDefault<CBaseCollisionObject2D*>(
													pCurrEntity->GetComponent<CBoxCollisionObject2D>(), 
													pCurrEntity->GetComponent<CCircleCollisionObject2D>()));
			
			pCurrBody->SetUserData(pCurrEntity);

			mCollidersData.mBodies.push_back(pCurrBody);
		}
	}

	void CPhysics2DSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CPhysics2DSystem::Update");

		mpWorldInstance->Step(mCurrTimeStep, mCurrVelocityIterations, mCurrPositionIterations);

		auto& collisionObjects = mCollidersData.mBodies;

		CTransform* pCurrTransform = nullptr;

		b2Body* pCurrBody = nullptr;

		TVector3 currPosition;

		b2Vec2 currBodyPosition;

		auto& transforms = mCollidersData.mTransforms;

		for (U32 i = 0; i < transforms.size(); ++i)
		{
			pCurrTransform = transforms[i];

			pCurrBody = mCollidersData.mBodies[i];
			
			currBodyPosition = pCurrBody->GetPosition();

			currPosition = pCurrTransform->GetPosition();

			currPosition.x = currBodyPosition.x;
			currPosition.y = currBodyPosition.y;

			pCurrTransform->SetPosition(currPosition);
		}
	}
	
	b2PolygonShape CPhysics2DSystem::CreateBoxCollisionShape(const CBoxCollisionObject2D& box) const
	{
		b2PolygonShape boxCollider;

		boxCollider.SetAsBox(box.GetWidth() * 0.5f, box.GetHeight() * 0.5f);
		
		return boxCollider;
	}

	b2CircleShape CPhysics2DSystem::CreateCircleCollisionShape(const CCircleCollisionObject2D& circle) const
	{
		b2CircleShape circleCollider;

		circleCollider.m_radius = circle.GetRadius();

		return circleCollider;
	}

	void CPhysics2DSystem::RaycastClosest(const TVector2& origin, const TVector2& direction, F32 maxDistance, const TOnRaycastHitCallback& onHitCallback)
	{
		TVector2 end = origin + (Length(direction) > 1e-3f ? Normalize(direction) : ZeroVector2) * maxDistance;

		if (Length(end - origin) < 1e-3f)
		{
			// \note the case of ray that's orthogonal for XY plane
			_testPointOverlap(origin, onHitCallback);
			return;
		}

		CRayCastClosestCallback callback(onHitCallback);

		mpWorldInstance->RayCast(&callback, { origin.x, origin.y }, { end.x, end.y });
	}

	bool CPhysics2DSystem::RaycastAll(const TVector2& origin, const TVector2& direction, F32 maxDistance, std::vector<TRaycastResult>& hitResults)
	{
		return false;
	}

	b2Body* CPhysics2DSystem::_createPhysicsBody(const CTransform* pTransform, bool isTrigger, const CBaseCollisionObject2D* pCollider)
	{
		TVector3 position = pTransform->GetPosition();
		TVector3 scale    = pTransform->GetScale();

		E_COLLISION_OBJECT_TYPE type = pCollider->GetCollisionType();

		b2BodyDef bodyDef;

		bodyDef.position.Set(position.x, position.y);

		switch (type)
		{
			case E_COLLISION_OBJECT_TYPE::COT_DYNAMIC:
				bodyDef.type = b2_dynamicBody;
				break;
			case E_COLLISION_OBJECT_TYPE::COT_STATIC:
				bodyDef.type = b2_staticBody;
				break;
			case E_COLLISION_OBJECT_TYPE::COT_KINEMATIC:
				bodyDef.type = b2_kinematicBody;
				break;
		}

		bodyDef.angle          = 0.0f;
		bodyDef.linearDamping  = 0.0f;
		bodyDef.angularDamping = 0.01f;
		bodyDef.gravityScale   = 1.0f;
		bodyDef.allowSleep     = true;
		bodyDef.awake          = true;
		bodyDef.active         = true;
				
		b2Body* pCreatedBody = mpWorldInstance->CreateBody(&bodyDef);

		if (!pCreatedBody)
		{
			return pCreatedBody;
		}

		b2FixtureDef fixtureDef;

		fixtureDef.friction = 0.3f;
		fixtureDef.density  = 1.0f;
		fixtureDef.isSensor = isTrigger;
		
		pCollider->GetCollisionShape(this, [&fixtureDef, &pCreatedBody](const b2Shape* pShapeCollider)
		{
			fixtureDef.shape = pShapeCollider;

			pCreatedBody->CreateFixture(&fixtureDef);
		}); /// this invokation creates a new fixture object

		return pCreatedBody;
	}

	void CPhysics2DSystem::_testPointOverlap(const TVector2& point, const TOnRaycastHitCallback& onHitCallback) const
	{
		CPointOverlapCallback callback;

		b2Vec2 p { point.x, point.y };
		b2AABB aabb;
		aabb.lowerBound = p;
		aabb.upperBound = p;

		mpWorldInstance->QueryAABB(&callback, aabb);

		TEntityId entityId = callback.GetEntityId();
		if (entityId != TEntityId::Invalid && onHitCallback)
		{
			onHitCallback({ entityId, { point.x, point.y, 0.0f }, ZeroVector3 });
		}
	}

	CPhysics2DSystem::CTriggerContactsListener::CTriggerContactsListener(IEventManager*& pEventManager, std::vector<b2Body*>& bodiesArray, 
																		 const THandles2EntitiesMap& handles2EntitiesMap):
		mpBodies(&bodiesArray), mpHandles2EntitiesMap(&handles2EntitiesMap), mpEventManager(pEventManager)
	{
	}

	void CPhysics2DSystem::CTriggerContactsListener::BeginContact(b2Contact* contact)
	{
		if (!contact->GetFixtureA()->IsSensor() && !contact->GetFixtureB()->IsSensor())
		{
			return;
		}

		TOnTrigger2DEnterEvent trigger2DEnterEventData;
		trigger2DEnterEventData.mEntities[0] = _getEntityIdByBody(contact->GetFixtureA()->GetBody());
		trigger2DEnterEventData.mEntities[1] = _getEntityIdByBody(contact->GetFixtureB()->GetBody());

		mpEventManager->Notify(&trigger2DEnterEventData);
	}

	void CPhysics2DSystem::CTriggerContactsListener::EndContact(b2Contact* contact)
	{
		if (!contact->GetFixtureA()->IsSensor() && !contact->GetFixtureB()->IsSensor())
		{
			return;
		}

		TOnTrigger2DExitEvent trigger2DExitEventData;
		trigger2DExitEventData.mEntities[0] = _getEntityIdByBody(contact->GetFixtureA()->GetBody());
		trigger2DExitEventData.mEntities[1] = _getEntityIdByBody(contact->GetFixtureB()->GetBody());

		mpEventManager->Notify(&trigger2DExitEventData);
	}

	CPhysics2DSystem::CRayCastClosestCallback::CRayCastClosestCallback(const TOnRaycastHitCallback& onHitCallback):
		mHit(false), mOnHitCallback(onHitCallback)
	{
	}

	F32 CPhysics2DSystem::CRayCastClosestCallback::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, F32 fraction)
	{
		b2Body* body = fixture->GetBody();
		
		TEntityId entityId = TEntityId::Invalid;

		if (auto pUserData = GetValidPtrOrDefault<void*>(body ? body->GetUserData() : nullptr, nullptr))
		{
			entityId = static_cast<CEntity*>(pUserData)->GetId();
		}

		mHit = true;

		mPoint  = point;
		mNormal = normal;

		if (mOnHitCallback)
		{
			mOnHitCallback({ entityId, { mPoint.x, mPoint.y, 0.0f }, { mNormal.x, mNormal.y, 0.0f } });
		}

		// By returning the current fraction, we instruct the calling code to clip the ray and
		// continue the ray-cast to the next fixture. WARNING: do not assume that fixtures
		// are reported in order. However, by clipping, we can always get the closest fixture.
		return fraction;
	}


	bool CPhysics2DSystem::CPointOverlapCallback::ReportFixture(b2Fixture* pFixture)
	{
		mpBody = pFixture ? pFixture->GetBody() : nullptr;
		return false;
	}

	TEntityId CPhysics2DSystem::CPointOverlapCallback::GetEntityId() const
	{
		if (auto pUserData = GetValidPtrOrDefault<void*>(mpBody ? mpBody->GetUserData() : nullptr, nullptr))
		{
			return static_cast<CEntity*>(pUserData)->GetId();
		}

		return TEntityId::Invalid;
	}

	TEntityId CPhysics2DSystem::CTriggerContactsListener::_getEntityIdByBody(const b2Body* pBody) const
	{
		auto iter = std::find(mpBodies->cbegin(), mpBodies->cend(), pBody);
		return (iter != mpBodies->cend()) ? static_cast<TEntityId>(mpHandles2EntitiesMap->at(static_cast<U32>(std::distance(mpBodies->cbegin(), iter)))) : TEntityId::Invalid;
	}


	TDE2_API ISystem* CreatePhysics2DSystem(IEventManager* pEventManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CPhysics2DSystem, result, pEventManager);
	}
}