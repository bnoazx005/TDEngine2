/*!
	\file CPhysics2DSystem.h
	\date 07.02.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseSystem.h"
#include "./../core/Event.h"
#include "./../physics/2D/ICollisionObjectsVisitor.h"
#include "./../math/TVector2.h"
#include "Box2D.h"


namespace TDEngine2
{
	class CTransform;
	class CBaseCollisionObject2D;
	class CBoxCollisionObject2D;
	class CCircleCollisionObject2D;
	class CTrigger2D;
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

			/*!
				class CContactsListener

				\brief The class implements a listener of all contacts
				that are occurs within b2World's instance
			*/

			class CTriggerContactsListener : public b2ContactListener
			{
				public:
					/// Called when two fixtures begin to touch.
					void BeginContact(b2Contact* contact) override;

					/// Called when two fixtures cease to touch.
					void EndContact(b2Contact* contact) override;
			};

			typedef TCollidersData<CBoxCollisionObject2D>    TBoxCollidersData;
			typedef TCollidersData<CCircleCollisionObject2D> TCircleCollidersData;
			typedef TCollidersData<CBaseCollisionObject2D>   TBaseCollidersData;
		public:
			/*!
				\brief The method initializes an inner state of a system

				\param[in, out] pEventManager A pointer to IEventManager implementation
		
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IEventManager* pEventManager);

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPhysics2DSystem)

			TDE2_API b2Body* _createPhysicsBody(const CTransform* pTransform, const CBaseCollisionObject2D* pCollider);
		protected:
			static const TVector2 mDefaultGravity;

			static const F32      mDefaultTimeStep;

			static const U32      mDefaultVelocityIterations;

			static const U32      mDefaultPositionIterations;

			b2World*              mpWorldInstance;

			b2ContactListener*    mpContactsListener;

			IEventManager*        mpEventManager;

			TBaseCollidersData    mCollidersData;

			TVector2              mCurrGravity;

			F32                   mCurrTimeStep;

			U32                   mCurrVelocityIterations;

			U32                   mCurrPositionIterations;
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

		// \todo
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

		// \todo
	} TOnTrigger2DExitEvent, *TOnTrigger2DExitEventPtr;
}