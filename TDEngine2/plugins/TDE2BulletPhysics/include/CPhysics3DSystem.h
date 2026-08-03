/*!
	\file CPhysics3DSystem.h
	\date 19.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include <ecs/CBaseSystem.h>
#include <physics/IPhysics3DSystem.h>
#include <physics/IRaycastContext.h>
#include <math/TVector3.h>
#include <core/Event.h>
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
class btMotionState;
class btGhostPairCallback;


namespace TDEngine2
{
	class CTransform;
	class CBaseCollisionObject3D;
	class CEntity;
	class IEventManager;
	class CPhysicsBody3D;


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

	class CPhysics3DSystem : public CBaseSystem, public IPhysics3DSystem
	{
		public:
			friend TDE2_API ISystem* CreatePhysics3DSystem(IEventManager* pEventManager, E_RESULT_CODE& result);
		protected:
			typedef struct TPhysicsObjectsData
			{
				std::vector<CTransform*>               mpTransforms;

				std::vector<CPhysicsBody3D*>           mpPhysicsBodies;

				std::vector<btCollisionShape*>         mpBulletColliderShapes;

				std::vector<btPairCachingGhostObject*> mpTriggers;

				std::vector<btCollisionObject*>        mpInternalCollisionObjects;

				std::vector<btMotionState*>            mpMotionHandlers;

				std::vector<bool>                      mInUseTable;

				TEntitiesArray                         mEntities;

				void Clear();

				void EraseItem(USIZE index);
			} TPhysicsObjectsData;

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

#if TDE2_EDITORS_ENABLED

			/*!
				\brief The method is targeted to draw debug information (textual or graphical) which is related with the given system
			*/

			TDE2_API void DebugOutput(IDebugUtility* pDebugUtility, F32 dt) const override;

#endif

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS_NO_DCTR(CPhysics3DSystem)
			TDE2_API virtual ~CPhysics3DSystem();

			E_RESULT_CODE _freePhysicsObjects(TPhysicsObjectsData& physicsData);

			E_RESULT_CODE _onFreeInternal() override;
		protected:
			IEventManager*                                       mpEventManager = nullptr;

			std::unique_ptr<btDefaultCollisionConfiguration>     mpCollisionConfiguration;
			std::unique_ptr<btCollisionDispatcher>               mpCollisionsDispatcher;
			std::unique_ptr<btBroadphaseInterface>               mpBroadphaseSolver;
			std::unique_ptr<btSequentialImpulseConstraintSolver> mpImpulseConstraintSolver;
			std::unique_ptr<btDiscreteDynamicsWorld>             mpWorld;			
			std::unique_ptr<btGhostPairCallback>                 mpGhostPairCallback;

			TVector3                                             mCurrGravity = ZeroVector3;

			F32                                                  mCurrTimeStep = 0.0f;

			U32                                                  mCurrPositionIterations = 0;

			TPhysicsObjectsData                                  mPhysicsObjectsData{};
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