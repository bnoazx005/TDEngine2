#include "./../../include/ecs/CPhysics2DSystem.h"
#include "./../../include/ecs/IWorld.h"
#include "./../../include/ecs/CEntity.h"
#include "./../../include/ecs/CTransform.h"
#include "./../../include/physics/2D/CBoxCollisionObject2D.h"
#include "./../../include/physics/2D/CCircleCollisionObject2D.h"


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
		std::vector<TEntityId> boxColliders = pWorld->FindEntitiesWithComponents<CTransform, CBoxCollisionObject2D>();
		
		mBoxCollidersData.Clear();

		CEntity* pCurrEntity = nullptr;

		CTransform* pTransform = nullptr;

		CBoxCollisionObject2D* pBoxCollisionObject = nullptr;

		for (auto iter = boxColliders.begin(); iter != boxColliders.end(); ++iter)
		{
			pCurrEntity = pWorld->FindEntity(*iter);

			if (!pCurrEntity)
			{
				continue;
			}

			pTransform = pCurrEntity->GetComponent<CTransform>();

			pBoxCollisionObject = pCurrEntity->GetComponent<CBoxCollisionObject2D>();

			mBoxCollidersData.mTransforms.push_back(pTransform);
			mBoxCollidersData.mCollisionObjects.push_back(pBoxCollisionObject);
			mBoxCollidersData.mBodies.push_back(_createBoxPhysicsBody(pTransform, pBoxCollisionObject));
		}

		std::vector<TEntityId> circleColliders = pWorld->FindEntitiesWithComponents<CTransform, CCircleCollisionObject2D>();

		mCircleCollidersData.Clear();
		
		for (auto iter = circleColliders.begin(); iter != circleColliders.end(); ++iter)
		{
			pCurrEntity = pWorld->FindEntity(*iter);

			if (!pCurrEntity)
			{
				continue;
			}

			mCircleCollidersData.mTransforms.push_back(pCurrEntity->GetComponent<CTransform>());
			mCircleCollidersData.mCollisionObjects.push_back(pCurrEntity->GetComponent<CCircleCollisionObject2D>());
		}
	}

	void CPhysics2DSystem::Update(IWorld* pWorld, F32 dt)
	{
		mpWorldInstance->Step(mCurrTimeStep, mCurrVelocityIterations, mCurrPositionIterations);

		auto boxCollisionObjects = mBoxCollidersData.mBodies;

		CTransform* pCurrTransform = nullptr;

		b2Body* pCurrBody = nullptr;

		TVector3 currPosition;

		b2Vec2 currBodyPosition;

		auto transforms = mBoxCollidersData.mTransforms;

		for (U32 i = 0; i < transforms.size(); ++i)
		{
			pCurrTransform = transforms[i];

			pCurrBody = mBoxCollidersData.mBodies[i];
			
			currBodyPosition = pCurrBody->GetPosition();

			currPosition = pCurrTransform->GetPosition();

			currPosition.x = currBodyPosition.x;
			currPosition.y = currBodyPosition.y;

			pCurrTransform->SetPosition(currPosition);
		}
	}

	b2Body* CPhysics2DSystem::_createBoxPhysicsBody(const CTransform* pTransform, const CBoxCollisionObject2D* pBoxCollider)
	{
		TVector3 position = pTransform->GetPosition();
		TVector3 scale    = pTransform->GetScale();

		E_COLLISION_OBJECT_TYPE type = pBoxCollider->GetCollisionType();

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

		b2PolygonShape boxCollider;
		boxCollider.SetAsBox(scale.x * 0.5f, scale.y * 0.5f);

		b2Body* pCreatedBody = mpWorldInstance->CreateBody(&bodyDef);

		if (!pCreatedBody)
		{
			return pCreatedBody;
		}

		b2FixtureDef fixtureDef;

		fixtureDef.shape    = &boxCollider;
		fixtureDef.friction = 0.3f;
		fixtureDef.density  = 1.0f;

		pCreatedBody->CreateFixture(&fixtureDef);

		return pCreatedBody;
	}

	b2Body* CPhysics2DSystem::_createCirclePhysicsBody(const CTransform* pTransform, const CCircleCollisionObject2D* pBoxCollider)
	{
		return nullptr;
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