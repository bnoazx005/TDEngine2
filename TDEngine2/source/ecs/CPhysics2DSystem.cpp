#include "../../include/ecs/CPhysics2DSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/ecs/CSystemManager.h"
#include "../../include/ecs/CTransformSystem.h"
#include "../../include/physics/2D/CBoxCollisionObject2D.h"
#include "../../include/physics/2D/CCircleCollisionObject2D.h"
#include "../../include/physics/2D/CTrigger2D.h"
#include "../../include/physics/CBaseRaycastContext.h"
#include "../../include/core/IEventManager.h"
#include "../../include/editor/CPerfProfiler.h"
#include "Box2D.h"
#include <algorithm>


namespace TDEngine2
{
	static const TVector2 DEFAULT_GRAVITY = TVector2(0.0f, -10.0f);

	TDE2_STATIC_CONSTEXPR const F32 DEFAULT_TIME_STEP = 1.0f / 60.0f;

	TDE2_STATIC_CONSTEXPR const U32 DEFAULT_VELOCITY_ITERATIONS = 6;

	TDE2_STATIC_CONSTEXPR const U32 DEFAULT_POSITION_ITERATIONS = 2;


	class CPointOverlapCallback : public b2QueryCallback
	{
		public:
			CPointOverlapCallback() = default;

			bool ReportFixture(b2Fixture* pFixture)
			{
				mpBody = pFixture ? pFixture->GetBody() : nullptr;
				return false;
			}

			TEntityId GetEntityId() const
			{
				if (auto pUserData = GetValidPtrOrDefault<void*>(mpBody ? mpBody->GetUserData() : nullptr, nullptr))
				{
					return static_cast<CEntity*>(pUserData)->GetId();
				}

				return TEntityId::Invalid;
			}
		private:
			b2Body* mpBody = nullptr;
	};


	class CRayCastClosestCallback : public b2RayCastCallback
	{
		public:
			CRayCastClosestCallback(const CPhysics2DSystem::TOnRaycastHitCallback& onHitCallback) :
				mOnHitCallback(onHitCallback)
			{
			}

			F32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, F32 fraction)
			{
				b2Body* body = fixture->GetBody();

				TEntityId entityId = TEntityId::Invalid;

				if (auto pUserData = GetValidPtrOrDefault<void*>(body ? body->GetUserData() : nullptr, nullptr))
				{
					entityId = static_cast<CEntity*>(pUserData)->GetId();
				}

				mHit = true;

				mPoint = point;
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
		private:
			bool                  mHit = false;

			b2Vec2                mPoint{};
			b2Vec2                mNormal{};

			CPhysics2DSystem::TOnRaycastHitCallback mOnHitCallback = nullptr;
	};


	/*!
		class CContactsListener

		\brief The class implements a listener of all contacts that are occurs within b2World's instance
	*/

	class CTriggerContactsListener : public b2ContactListener
	{
		public:
			CTriggerContactsListener(IEventManager*& pEventManager, std::vector<b2Body*>& bodiesArray, const CPhysics2DSystem::THandles2EntitiesMap& handles2EntitiesMap):
				mpBodies(&bodiesArray), mpHandles2EntitiesMap(&handles2EntitiesMap), mpEventManager(pEventManager)
			{
			}

			/// Called when two fixtures begin to touch.
			void BeginContact(b2Contact* contact)
			{
				if (!contact->GetFixtureA()->IsSensor() && !contact->GetFixtureB()->IsSensor())
				{
					return;
				}

				TOnTrigger2DEnterEvent trigger2DEnterEventData;
				trigger2DEnterEventData.mEntities[0] = _getEntityIdByBody(contact->GetFixtureA()->GetBody());
				trigger2DEnterEventData.mEntities[1] = _getEntityIdByBody(contact->GetFixtureB()->GetBody());

				mpEventManager->Notify(trigger2DEnterEventData);
			}

			/// Called when two fixtures cease to touch.
			void EndContact(b2Contact* contact)
			{
				if (!contact->GetFixtureA()->IsSensor() && !contact->GetFixtureB()->IsSensor())
				{
					return;
				}

				TOnTrigger2DExitEvent trigger2DExitEventData;
				trigger2DExitEventData.mEntities[0] = _getEntityIdByBody(contact->GetFixtureA()->GetBody());
				trigger2DExitEventData.mEntities[1] = _getEntityIdByBody(contact->GetFixtureB()->GetBody());

				mpEventManager->Notify(trigger2DExitEventData);
			}

		private:
			TEntityId _getEntityIdByBody(const b2Body* pBody) const
			{
				auto iter = std::find(mpBodies->cbegin(), mpBodies->cend(), pBody);
				return (iter != mpBodies->cend()) ? static_cast<TEntityId>(mpHandles2EntitiesMap->at(static_cast<U32>(std::distance(mpBodies->cbegin(), iter)))) : TEntityId::Invalid;
			}

		private:
			std::vector<b2Body*>*                         mpBodies = nullptr;
			const CPhysics2DSystem::THandles2EntitiesMap* mpHandles2EntitiesMap;
			IEventManager*                                mpEventManager = nullptr;
	};


	CPhysics2DSystem::CPhysics2DSystem() :
		CBaseSystem(), mpWorldInstance(nullptr)
	{
	}

	E_RESULT_CODE CPhysics2DSystem::Init(IEventManager* pEventManager)
	{
		TDE2_PROFILER_SCOPE("CPhysics2DSystem::Init");

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mpWorldInstance    = std::make_unique<b2World>(b2Vec2{ DEFAULT_GRAVITY.x, DEFAULT_GRAVITY.y });
		mpContactsListener = std::make_unique<CTriggerContactsListener>(pEventManager, mCollidersData.mBodies, mHandles2EntitiesMap);

		mpWorldInstance->SetContactListener(mpContactsListener.get());

		mpEventManager = pEventManager;

		mCurrGravity = DEFAULT_GRAVITY;

		mCurrTimeStep = DEFAULT_TIME_STEP;

		mCurrVelocityIterations = DEFAULT_VELOCITY_ITERATIONS;
		mCurrPositionIterations = DEFAULT_POSITION_ITERATIONS;

		mIsInitialized = true;

		return RC_OK;
	}


	static b2Body* CreatePhysicsBody(b2World* pWorld, ICollisionObjectsVisitor* pCollisionObjectsVisitor, const CTransform* pTransform, bool isTrigger, const CBaseCollisionObject2D* pCollider)
	{
		TVector3 position = pTransform->GetPosition();
		TVector3 scale = pTransform->GetScale();

		b2BodyDef bodyDef;

		bodyDef.position.Set(position.x, position.y);

		switch (pCollider->GetCollisionType())
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

		b2Body* pCreatedBody = pWorld->CreateBody(&bodyDef);
		if (!pCreatedBody)
		{
			return pCreatedBody;
		}

		b2FixtureDef fixtureDef;

		fixtureDef.friction = 0.3f;
		fixtureDef.density = 1.0f;
		fixtureDef.isSensor = isTrigger;

		pCollider->GetCollisionShape(pCollisionObjectsVisitor, [&fixtureDef, &pCreatedBody](const b2Shape* pShapeCollider)
			{
				fixtureDef.shape = pShapeCollider;

				pCreatedBody->CreateFixture(&fixtureDef);
			}); /// this invokation creates a new fixture object

		return pCreatedBody;
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

			pCurrBody = CreatePhysicsBody(mpWorldInstance.get(), this, pTransform, pCurrTrigger,
										   GetValidPtrOrDefault<CBaseCollisionObject2D*>(pCurrEntity->GetComponent<CBoxCollisionObject2D>(), pCurrEntity->GetComponent<CCircleCollisionObject2D>()));
			
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


	static void TestPointOverlap(b2World* pWorld, const TVector2& point, const CPhysics2DSystem::TOnRaycastHitCallback& onHitCallback)
	{
		CPointOverlapCallback callback;

		b2Vec2 p{ point.x, point.y };
		b2AABB aabb;
		aabb.lowerBound = p;
		aabb.upperBound = p;

		pWorld->QueryAABB(&callback, aabb);

		TEntityId entityId = callback.GetEntityId();
		if (entityId != TEntityId::Invalid && onHitCallback)
		{
			onHitCallback({ entityId, { point.x, point.y, 0.0f }, ZeroVector3 });
		}
	}


	void CPhysics2DSystem::RaycastClosest(const TVector2& origin, const TVector2& direction, F32 maxDistance, const TOnRaycastHitCallback& onHitCallback)
	{
		TVector2 end = origin + (Length(direction) > 1e-3f ? Normalize(direction) : ZeroVector2) * maxDistance;

		if (Length(end - origin) < 1e-3f)
		{
			// \note the case of ray that's orthogonal for XY plane
			TestPointOverlap(mpWorldInstance.get(), origin, onHitCallback);
			return;
		}

		CRayCastClosestCallback callback(onHitCallback);

		mpWorldInstance->RayCast(&callback, { origin.x, origin.y }, { end.x, end.y });
	}

	bool CPhysics2DSystem::RaycastAll(const TVector2& origin, const TVector2& direction, F32 maxDistance, std::vector<TRaycastResult>& hitResults)
	{
		return false;
	}


	TDE2_API ISystem* CreatePhysics2DSystem(IEventManager* pEventManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CPhysics2DSystem, result, pEventManager);
	}


	struct TPhysics2DSystemAutoInitializer : TSystemAutoInitializer
	{
		virtual ~TPhysics2DSystemAutoInitializer() = default;

		E_RESULT_CODE ManageDependencies(IWorld* pWorld) override
		{
			TSystemId transformSystemHandle = pWorld->FindSystem<CTransformSystem>();
			if (transformSystemHandle == TSystemId::Invalid)
			{
				return RC_FAIL;
			}

			return pSystemInstance ? pSystemInstance->AddDependency(pWorld->GetSystem(transformSystemHandle)) : RC_FAIL;
		}

		ISystem* GetSystem(IWorld* pWorld, const TRequestSubsystemCallback& subsystemsProviderCallback)
		{
			E_RESULT_CODE result = RC_OK;

			pSystemInstance = CreatePhysics2DSystem(PolymorphicCast<IEventManager*>(subsystemsProviderCallback(E_ENGINE_SUBSYSTEM_TYPE::EST_EVENT_MANAGER)), result);

			pWorld->RegisterRaycastContext(TPtr<IRaycastContext>(CreateBaseRaycastContext(dynamic_cast<CPhysics2DSystem*>(pSystemInstance), nullptr, result)));

			return pSystemInstance;
		}

		ISystem* pSystemInstance = nullptr;
	};


	TDE2_REGISTER_SYSTEM(TPhysics2DSystemAutoInitializer);
}