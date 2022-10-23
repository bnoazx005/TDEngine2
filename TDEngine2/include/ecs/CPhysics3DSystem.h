/*!
	\file CPhysics3DSystem.h
	\date 19.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseSystem.h"
#include "../physics/3D/ICollisionObjects3DVisitor.h"
#include "../physics/IRaycastContext.h"
#include "../math/TVector3.h"
#include "../core/Event.h"
#include "../../deps/bullet3/src/LinearMath/btMotionState.h"
#include <vector>
#include <functional>


// Bullet3's forward declarations
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btCollisionShape;
class btRigidBody;
class btCollisionObject;
class btPairCachingGhostObject;


namespace TDEngine2
{
	class CTransform;
	class CBaseCollisionObject3D;
	class CEntity;
	class IEventManager;


	/*!
		\brief A factory function for creation objects of CPhysics3DSystem's type.

		\param[in, out] pEventManager A pointer to IEventManager implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CPhysics3DSystem's implementation
	*/

	TDE2_API ISystem* CreatePhysics3DSystem(IEventManager* pEventManager, E_RESULT_CODE& result);


	/*!
		class CPhysics3DSystem

		\brief The system implements an update step of 3D physics engine
	*/

	class CPhysics3DSystem : public CBaseSystem, public ICollisionObjects3DVisitor
	{
		public:
			friend TDE2_API ISystem* CreatePhysics3DSystem(IEventManager* pEventManager, E_RESULT_CODE& result);
		protected:
			typedef struct TPhysicsObjectsData
			{
				std::vector<CTransform*>             mpTransforms;

				std::vector<CBaseCollisionObject3D*> mpCollisionObjects;

				std::vector<btCollisionShape*>       mpBulletColliderShapes;

				std::vector<btPairCachingGhostObject*> mpTriggers;

				std::vector<btCollisionObject*>      mpInternalCollisionObjects;

				std::vector<btMotionState*>          mpMotionHandlers;

				std::vector<bool>                    mInUseTable;

				void Clear();

				void EraseItem(USIZE index);
			} TPhysicsObjectsData;

			/// \fixme Replace this directive with alignas when corresponding functionality will be supported in tde2_introspector
#pragma pack(push, 16)
			struct /*alignas(16) */TEntitiesMotionState : public btMotionState
			{
				btTransform mGraphicsWorldTrans;
				btTransform mCenterOfMassOffset;
				btTransform mStartWorldTrans;
				
				void*       mUserPointer;

				CTransform* mpEntityTransform; // \todo Replace with entity id and pointer to CWorld*
				
				BT_DECLARE_ALIGNED_ALLOCATOR();

				TEntitiesMotionState(CTransform* pEntityTransform, const btTransform& startTrans = btTransform::getIdentity(), const btTransform& centerOfMassOffset = btTransform::getIdentity());

				TDE2_API void getWorldTransform(btTransform & centerOfMassWorldTrans) const override;
				TDE2_API void setWorldTransform(const btTransform& centerOfMassWorldTrans) override;
			};

#pragma pack(pop)

			typedef std::function<void(const TRaycastResult&)> TOnRaycastHitCallback;
		public:
			TDE2_SYSTEM(CPhysics3DSystem);

			/*!
				\brief The method initializes an inner state of a system

				\param[in, out] pEventManager A pointer to IEventManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IEventManager* pEventManager);

			/*!
				\brief The method inject components array into a system

				\param[in] pWorld A pointer to a main scene's object
			*/

			TDE2_API void InjectBindings(IWorld* pWorld) override;

			/*!
				\brief The main method that should be implemented in all derived classes.
				It contains all the logic that the system will execute during engine's work.

				\param[in] pWorld A pointer to a main scene's object

				\param[in] dt A delta time's value
			*/

			TDE2_API void Update(IWorld* pWorld, F32 dt) override;

			/*!
				\brief The method returns a new created collision shape which is a box collider

				\param[in] box A reference to a box collision object

				\return The method returns a new created collision shape of a box collider
			*/

			TDE2_API btBoxShape* CreateBoxCollisionShape(const CBoxCollisionObject3D& box) const override;

			/*!
				\brief The method returns a new created collision shape which is a sphere collider

				\param[in] sphere A reference to a sphere collision object

				\return The method returns a new created collision shape of a sphere collider
			*/

			TDE2_API btSphereShape* CreateSphereCollisionShape(const CSphereCollisionObject3D& sphere) const override;

			/*!
				\brief The method returns a new created collision shape which is a convex hull. This is handful for
				approximations of triangle meshes

				\param[in] hull A reference to a convex hull's object

				\return The method returns a new created collision shape of a convex hull
			*/

			TDE2_API btConvexHullShape* CreateConvexHullCollisionShape(const CConvexHullCollisionObject3D& hull) const override;

			/*!
				\brief The method casts a ray into a scene and returns closest object which is intersected by that.
				If there wasn't intersections nullptr is returned. The method isn't asynchronous, its callback will
				be called before the method returns execution context to its caller

				\param[in] origin A position in world space of a ray's origin
				\param[in] direction A normalized direction of a ray
				\param[in] maxDistance A maximal distance of a ray through given direction
				\param[in] onHitCallback A callback which is called only if some object was hitted by the ray

				\return The method casts a ray into a scene and returns closest object which is intersected by that.
				If there wasn't intersections nullptr is returned
				*/
			TDE2_API void RaycastClosest(const TVector3& origin, const TVector3& direction, F32 maxDistance, const TOnRaycastHitCallback& onHitCallback);

			/*!
				\brief The method casts a ray into the world and gathers all collision objects that intersect with the it

				\param[in] origin A position in world space of a ray's origin
				\param[in] direction A normalized direction of a ray
				\param[in] maxDistance A maximal distance of a ray through given direction
				\param[out] result An array with TRaycastResult objects

				\return The method returns true if some intersections were found, false in other cases
			*/

			TDE2_API bool RaycastAll(const TVector3& origin, const TVector3& direction, F32 maxDistance, std::vector<TRaycastResult>& hitResults);
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPhysics3DSystem)

			TDE2_API std::tuple<btRigidBody*, btMotionState*> _createRigidbody(const CBaseCollisionObject3D& collisionObject, CTransform* pTransform, btCollisionShape* pColliderShape) const;

			TDE2_API std::tuple<btPairCachingGhostObject*, btMotionState*> _createTrigger(const CBaseCollisionObject3D& collisionObject, CTransform* pTransform, btCollisionShape* pColliderShape) const;

			TDE2_API E_RESULT_CODE _freePhysicsObjects(TPhysicsObjectsData& physicsData);

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			static const TVector3                mDefaultGravity;

			static const F32                     mDefaultTimeStep;

			static const U32                     mDefaultPositionIterations;

			IEventManager*                       mpEventManager;

			btDefaultCollisionConfiguration*     mpCollisionConfiguration;

			btCollisionDispatcher*               mpCollisionsDispatcher;

			btBroadphaseInterface*               mpBroadphaseSolver;

			btSequentialImpulseConstraintSolver* mpImpulseConstraintSolver;

			btDiscreteDynamicsWorld*             mpWorld;

			TVector3                             mCurrGravity;

			F32                                  mCurrTimeStep;

			U32                                  mCurrPositionIterations;

			TPhysicsObjectsData                  mPhysicsObjectsData;
	};


	/*!
		struct TOnTrigger3DEvent

		\brief The structure represents an event which occurs
		when some entity enters into a 3D trigger
	*/

	typedef struct TOnTrigger3DEvent : TBaseEvent
	{
		virtual ~TOnTrigger3DEvent() = default;

		TDE2_REGISTER_TYPE(TOnTrigger3DEvent)
		REGISTER_EVENT_TYPE(TOnTrigger3DEvent)

		TEntityId mEntities[2]; ///< Two bodies that were collided
	} TOnTrigger3DEvent, *TOnTrigger3DEventPtr;


	/*!
		struct TOn3DCollisionRegistered

		\brief The structure represents an event which occurs when two or more 3d physical objects are collided 
	*/

	typedef struct TOn3DCollisionRegisteredEvent : TBaseEvent
	{
		enum class E_COLLISION_EVENT_TYPE : U8
		{
			ON_ENTER,
			ON_STAY,
			ON_EXIT
		};

		virtual ~TOn3DCollisionRegisteredEvent() = default;

		TDE2_REGISTER_TYPE(TOn3DCollisionRegisteredEvent)
		REGISTER_EVENT_TYPE(TOn3DCollisionRegisteredEvent)

		TEntityId              mEntities[2]; ///< Two bodies that were collided
		TVector3               mContactNormal;
		E_COLLISION_EVENT_TYPE mType;
	} TOn3DCollisionRegisteredEvent, *TOn3DCollisionRegisteredEventPtr;
}