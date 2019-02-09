/*!
	\file CPhysics2DSystem.h
	\date 07.02.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "ISystem.h"
#include "./../core/CBaseObject.h"
#include "./../math/TVector2.h"
#include "Box2D.h"


namespace TDEngine2
{
	class CTransform;
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

	class CPhysics2DSystem: public ISystem, public CBaseObject
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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPhysics2DSystem)

			TDE2_API b2Body* _createBoxPhysicsBody(const CTransform* pTransform, const CBoxCollisionObject2D* pBoxCollider);

			TDE2_API b2Body* _createCirclePhysicsBody(const CTransform* pTransform, const CCircleCollisionObject2D* pBoxCollider);
		protected:
			static const TVector2    mDefaultGravity;

			static const F32         mDefaultTimeStep;

			static const U32         mDefaultVelocityIterations;

			static const U32         mDefaultPositionIterations;

			b2World*                 mpWorldInstance;

			TBoxCollidersData        mBoxCollidersData;

			TCircleCollidersData     mCircleCollidersData;

			TVector2                 mCurrGravity;

			F32                      mCurrTimeStep;

			U32                      mCurrVelocityIterations;

			U32                      mCurrPositionIterations;
	};
}