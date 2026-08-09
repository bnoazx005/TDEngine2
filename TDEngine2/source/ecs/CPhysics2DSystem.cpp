#include "../../include/ecs/CPhysics2DSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/ecs/CSystemManager.h"
#include "../../include/ecs/CTransformSystem.h"
#include "../../include/physics/2D/CBoxCollisionObject2D.h"
#include "../../include/physics/2D/CCircleCollisionObject2D.h"
#include "../../include/physics/2D/CTrigger2D.h"
#include "../../include/physics/2D/CPhysicsBody2D.h"
#include "../../include/physics/CBaseRaycastContext.h"
#include "../../include/core/IEventManager.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/math/MathUtils.h"
#include "Box2D.h"
#include <algorithm>


namespace TDEngine2
{
	static const TVector2 DEFAULT_GRAVITY = TVector2(0.0f, -10.0f);

	TDE2_STATIC_CONSTEXPR F32 DEFAULT_TIME_STEP           = 1.0f / 60.0f;
	TDE2_STATIC_CONSTEXPR U32 DEFAULT_VELOCITY_ITERATIONS = 6;
	TDE2_STATIC_CONSTEXPR U32 DEFAULT_POSITION_ITERATIONS = 2;


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
			CTriggerContactsListener(IEventManager*& pEventManager):
				mpEventManager(pEventManager)
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
				trigger2DEnterEventData.mEntities[0] = static_cast<CTransform*>(contact->GetFixtureA()->GetUserData())->GetOwnerId();
				trigger2DEnterEventData.mEntities[1] = static_cast<CTransform*>(contact->GetFixtureB()->GetUserData())->GetOwnerId();

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
				trigger2DExitEventData.mEntities[0] = static_cast<CTransform*>(contact->GetFixtureA()->GetUserData())->GetOwnerId();
				trigger2DExitEventData.mEntities[1] = static_cast<CTransform*>(contact->GetFixtureB()->GetUserData())->GetOwnerId();

				mpEventManager->Notify(trigger2DExitEventData);
			}

		private:
			IEventManager* mpEventManager = nullptr;
	};


	CPhysics2DSystem::CPhysics2DSystem() :
		CBaseSystem(), mpWorldInstance(nullptr), mpContactsListener(nullptr)
	{
	}

	CPhysics2DSystem::~CPhysics2DSystem()
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
		mpContactsListener = std::make_unique<CTriggerContactsListener>(pEventManager);

		mpWorldInstance->SetContactListener(mpContactsListener.get());

		mpEventManager = pEventManager;

		mCurrGravity = DEFAULT_GRAVITY;

		mCurrTimeStep = DEFAULT_TIME_STEP;

		mCurrVelocityIterations = DEFAULT_VELOCITY_ITERATIONS;
		mCurrPositionIterations = DEFAULT_POSITION_ITERATIONS;

		mIsInitialized = true;

		return RC_OK;
	}


	template <typename TCollisionType, typename TFunctor>
	static void CreateCollisionShape(const TCollisionType& collisionObject, TFunctor&& postAction)
	{
	}


	template <typename TFunctor>
	static void CreateCollisionShape(const CBoxCollisionObject2D& box, TFunctor&& postAction)
	{
		b2PolygonShape boxCollider;

		boxCollider.SetAsBox(box.GetData().mWidth * 0.5f, box.GetData().mHeight * 0.5f);
		postAction(&boxCollider);
	}


	template <typename TFunctor>
	static void CreateCollisionShape(const CCircleCollisionObject2D& circle, TFunctor&& postAction)
	{
		b2CircleShape circleCollider;

		circleCollider.m_radius = circle.GetData().mRadius;
		postAction(&circleCollider);
	}


	template <typename TCollisionObjectType>
	static b2Body* CreatePhysicsBody(b2World* pWorld, CTransform* pTransform, bool isTrigger, CPhysicsBody2D* pPhysicsBody, const TCollisionObjectType* pCollider)
	{
		TVector3 position = pTransform->GetPosition();
		TVector3 scale = pTransform->GetScale();

		b2BodyDef bodyDef;

		bodyDef.position.Set(position.x, position.y);

		const TPhysicsBody2DComponentData& physicsObjectData = pPhysicsBody->GetData();
		
		switch (physicsObjectData.mType)
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
		fixtureDef.density  = 1.0f;
		fixtureDef.isSensor = isTrigger;

		CreateCollisionShape(*pCollider, [&fixtureDef, &pCreatedBody, pTransform](const b2Shape* pShapeCollider)
			{
				fixtureDef.shape = pShapeCollider;

				b2Fixture* pFixture = pCreatedBody->CreateFixture(&fixtureDef);
				pFixture->SetUserData(pTransform);
			}); /// this invokation creates a new fixture object

		return pCreatedBody;
	}


	template <typename TCollisionObjectType>
	static void InitCollidersContext(b2World* pPhysicsWorld, IWorld* pWorld, CPhysics2DSystem::TCollidersData<TCollisionObjectType>& context, const TEntitiesArray& entities)
	{
		/// Remove all bodies from the world's instance
		for (auto pCurrBody : context.mpBodies)
		{
			if (!pCurrBody)
			{
				continue;
			}

			pPhysicsWorld->DestroyBody(pCurrBody);
		}

		context.Clear();

		CEntity* pCurrEntity = nullptr;

		b2Body* pCurrBody = nullptr;

		for (TEntityId currEntityId : entities)
		{
			if (!(pCurrEntity = pWorld->FindEntity(currEntityId)))
			{
				continue;
			}

			context.mpTransforms.push_back(pCurrEntity->GetComponent<CTransform>());
			context.mpCollisionObjects.push_back(pCurrEntity->GetComponent<TCollisionObjectType>());
			context.mpPhysBodies.push_back(pCurrEntity->GetComponent<CPhysicsBody2D>());
			context.mpTriggers.push_back(pCurrEntity->GetComponent<CTrigger2D>());

			pCurrBody = CreatePhysicsBody(pPhysicsWorld, context.mpTransforms.back(), context.mpTriggers.back(), context.mpPhysBodies.back(), context.mpCollisionObjects.back());
			pCurrBody->SetUserData(pCurrEntity);

			context.mpBodies.push_back(pCurrBody);
		}
	}


	void CPhysics2DSystem::InjectBindings(IWorld* pWorld)
	{		
		InitCollidersContext(mpWorldInstance.get(), pWorld, mBoxCollidersData, pWorld->FindEntitiesWithComponents<CBoxCollisionObject2D, CPhysicsBody2D>());
		InitCollidersContext(mpWorldInstance.get(), pWorld, mCircleCollidersData, pWorld->FindEntitiesWithComponents<CCircleCollisionObject2D, CPhysicsBody2D>());
	}


	template <typename TCollisionObjectType>
	static void UpdateTransforms(CPhysics2DSystem::TCollidersData<TCollisionObjectType>& context)
	{
		CTransform* pCurrTransform = nullptr;

		b2Body* pCurrBody = nullptr;

		TVector3 currPosition;

		b2Vec2 currBodyPosition;

		auto& transforms = context.mpTransforms;

		for (U32 i = 0; i < transforms.size(); ++i)
		{
			pCurrTransform = transforms[i];

			pCurrBody = context.mpBodies[i];

			currBodyPosition = pCurrBody->GetPosition();

			currPosition = pCurrTransform->GetPosition();

			currPosition.x = currBodyPosition.x;
			currPosition.y = currBodyPosition.y;

			pCurrTransform->SetPosition(currPosition);
		}
	}


	void CPhysics2DSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CPhysics2DSystem::Update");

		mpWorldInstance->Step(mCurrTimeStep, mCurrVelocityIterations, mCurrPositionIterations);

		UpdateTransforms(mBoxCollidersData);
		UpdateTransforms(mCircleCollidersData);
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
		TVector2 end = origin + (Length(direction) > FloatEpsilon ? Normalize(direction) : ZeroVector2) * maxDistance;

		if (Length(end - origin) < FloatEpsilon)
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