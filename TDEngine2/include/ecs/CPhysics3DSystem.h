/*!
	\file CPhysics3DSystem.h
	\date 19.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseSystem.h"
#include "./../math/TVector3.h"
#include "./../core/Event.h"
#include <vector>
#include <unordered_map>


namespace TDEngine2
{
	class CTransform;
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

	class CPhysics3DSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreatePhysics3DSystem(IEventManager* pEventManager, E_RESULT_CODE& result);
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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPhysics3DSystem)
		protected:
			static const TVector3 mDefaultGravity;

			static const F32      mDefaultTimeStep;

			static const U32      mDefaultVelocityIterations;

			static const U32      mDefaultPositionIterations;

			IEventManager*        mpEventManager;

			TVector3              mCurrGravity;

			F32                   mCurrTimeStep;

			U32                   mCurrVelocityIterations;

			U32                   mCurrPositionIterations;
	};
}