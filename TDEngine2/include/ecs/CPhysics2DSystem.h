/*!
	\file CPhysics2DSystem.h
	\date 07.02.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseSystem.h"
#include "../core/Event.h"
#include "../physics/2D/ICollisionObjectsVisitor.h"
#include "../physics/IRaycastContext.h"
#include "../math/TVector2.h"
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>


class b2World;
class b2ContactListener;
class b2Body;


namespace TDEngine2
{
	class CTransform;
	class CBaseCollisionObject2D;
	class CBoxCollisionObject2D;
	class CCircleCollisionObject2D;
	class CTrigger2D;
	class CEntity;
	class IEventManager;


	/*!
		\brief A factory function for creation objects of CPhysics2DSystem's type.

		\param[in, out] pEventManager A pointer to IEventManager implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CPhysics2DSystem's implementation
	*/

	TDE2_API ISystem* CreatePhysics2DSystem(IEventManager* pEventManager, E_RESULT_CODE& result);


	/*!
		class CPhysics2DSystem

		\brief The system implements an update step of 2D physics engine
	*/

	class CPhysics2DSystem: public CBaseSystem, public ICollisionObjectsVisitor
	{
		public:
			friend TDE2_API ISystem* CreatePhysics2DSystem(IEventManager* pEventManager, E_RESULT_CODE& result);
		public:
			template <typename T>
			struct TCollidersData
			{
				std::vector<CTransform*> mTransforms;

				std::vector<T*>          mCollisionObjects;

				std::vector<CTrigger2D*> mTriggers;

				std::vector<b2Body*>     mBodies;

				void Clear()
				{
					mTransforms.clear();
					mCollisionObjects.clear();
					mBodies.clear();
					mTriggers.clear();
				}
			};

			typedef std::unordered_map<U32, TEntityId>     THandles2EntitiesMap;
			typedef TCollidersData<CBaseCollisionObject2D> TBaseCollidersData;

			typedef std::function<void(const TRaycastResult&)> TOnRaycastHitCallback;
		public:
			TDE2_SYSTEM(CPhysics2DSystem);

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

				\return The method returns a new created collision shape which is a box collider
			*/

			TDE2_API b2PolygonShape CreateBoxCollisionShape(const CBoxCollisionObject2D& box) const override;

			/*!
				\brief The method returns a new created collision shape which is a circle collider

				\param[in] circle A reference to a circle collision object

				\return The method returns a new created collision shape which is a circle collider
			*/

			TDE2_API b2CircleShape CreateCircleCollisionShape(const CCircleCollisionObject2D& circle) const override;

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
			TDE2_API void RaycastClosest(const TVector2& origin, const TVector2& direction, F32 maxDistance, const TOnRaycastHitCallback& onHitCallback);

			/*!
				\brief The method casts a ray into the world and gathers all collision objects that intersect with the it

				\param[in] origin A position in world space of a ray's origin
				\param[in] direction A normalized direction of a ray
				\param[in] maxDistance A maximal distance of a ray through given direction
				\param[out] result An array with TRaycastResult objects

				\return The method returns true if some intersections were found, false in other cases
			*/

			TDE2_API bool RaycastAll(const TVector2& origin, const TVector2& direction, F32 maxDistance, std::vector<TRaycastResult>& hitResults);
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPhysics2DSystem)
		protected:
			std::unique_ptr<b2World>           mpWorldInstance = nullptr;

			std::unique_ptr<b2ContactListener> mpContactsListener = nullptr;

			IEventManager*        mpEventManager = nullptr;

			THandles2EntitiesMap  mHandles2EntitiesMap;

			TBaseCollidersData    mCollidersData;

			TVector2              mCurrGravity;

			F32                   mCurrTimeStep = 0.0f;

			U32                   mCurrVelocityIterations = 0;
			U32                   mCurrPositionIterations = 0;
	};


	/*!
		struct TOnTrigger2DEnterEvent

		\brief The structure represents an event which occurs
		when some entity enters into a 2D trigger
	*/

	typedef struct TOnTrigger2DEnterEvent : TBaseEvent
	{
		virtual ~TOnTrigger2DEnterEvent() = default;

		TDE2_REGISTER_TYPE(TOnTrigger2DEnterEvent)
		REGISTER_EVENT_TYPE(TOnTrigger2DEnterEvent)

		TEntityId mEntities[2]{ TEntityId::Invalid, TEntityId::Invalid }; ///< Two bodies that were collided
	} TOnTrigger2DEnterEvent, *TOnTrigger2DEnterEventPtr;


	/*!
		struct TOnTrigger2DExitEvent

		\brief The structure represents an event which occurs
		when some entity exits into a 2D trigger
	*/

	typedef struct TOnTrigger2DExitEvent : TBaseEvent
	{
		virtual ~TOnTrigger2DExitEvent() = default;

		TDE2_REGISTER_TYPE(TOnTrigger2DExitEvent)
		REGISTER_EVENT_TYPE(TOnTrigger2DExitEvent)

		TEntityId mEntities[2]{ TEntityId::Invalid, TEntityId::Invalid }; ///< Two bodies that were collided
	} TOnTrigger2DExitEvent, *TOnTrigger2DExitEventPtr;
}