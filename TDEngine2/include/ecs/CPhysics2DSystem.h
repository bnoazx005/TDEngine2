/*!
	\file CPhysics2DSystem.h
	\date 07.02.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "ISystem.h"
#include "./../physics/2D/ICollisionObjectsVisitor.h"
#include "./../core/CBaseObject.h"
#include "./../math/TVector2.h"
#include "Box2D.h"


namespace TDEngine2
{
	class CTransform;
	class CBaseCollisionObject2D;
	class CBoxCollisionObject2D;
	class CCircleCollisionObject2D;


	/*!
		\brief A factory function for creation objects of CPhysics2DSystem's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CPhysics2DSystem's implementation
	*/

	TDE2_API ISystem* CreatePhysics2DSystem(E_RESULT_CODE& result);


	/*!
		class CPhysics2DSystem

		\brief The system implements an update step of 2D physics engine
	*/

	class CPhysics2DSystem: public ISystem, public ICollisionObjectsVisitor, public CBaseObject
	{
		public:
			friend TDE2_API ISystem* CreatePhysics2DSystem(E_RESULT_CODE& result);
		public:
			template <typename T>
			struct TCollidersData
			{
				std::vector<CTransform*> mTransforms;

				std::vector<T*>          mCollisionObjects;

				std::vector<b2Body*>     mBodies;

				void Clear()
				{
					mTransforms.clear();
					mCollisionObjects.clear();
					mBodies.clear();
				}
			};

			typedef TCollidersData<CBoxCollisionObject2D>    TBoxCollidersData;
			typedef TCollidersData<CCircleCollisionObject2D> TCircleCollidersData;
			typedef TCollidersData<CBaseCollisionObject2D>   TBaseCollidersData;
		public:
			/*!
				\brief The method initializes an inner state of a system

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init();

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
			static const TVector2    mDefaultGravity;

			static const F32         mDefaultTimeStep;

			static const U32         mDefaultVelocityIterations;

			static const U32         mDefaultPositionIterations;

			b2World*                 mpWorldInstance;

			TBaseCollidersData       mCollidersData;

			TVector2                 mCurrGravity;

			F32                      mCurrTimeStep;

			U32                      mCurrVelocityIterations;

			U32                      mCurrPositionIterations;
	};
}