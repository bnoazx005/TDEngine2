/*!
	\file CPhysics3DSystem.h
	\date 19.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseSystem.h"
#include "./../../include/physics/3D/ICollisionObjects3DVisitor.h"
#include "./../math/TVector3.h"
#include "./../core/Event.h"
#include <vector>


// Bullet3's forward declarations
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btCollisionShape;
class btRigidBody;


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

				std::vector<btRigidBody*>            mpRigidBodies;

				void Clear();
			} TPhysicsObjectsData;
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

				\return The method returns a new created collision shape of a box collider
			*/

			TDE2_API btBoxShape* CreateBoxCollisionShape(const CBoxCollisionObject3D& box) const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPhysics3DSystem)

			TDE2_API btRigidBody* _createRigidbody(const CBaseCollisionObject3D& collisionObject, btCollisionShape* pColliderShape) const;
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
}