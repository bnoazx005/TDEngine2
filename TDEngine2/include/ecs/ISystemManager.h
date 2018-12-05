/*!
	\file ISystemManager.h
	\date 29.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/IBaseObject.h"
#include "./../utils/Utils.h"


namespace TDEngine2
{
	class ISystem;
	class IWorld;
	class IEventManager;


	/*!
		interface ISystemManager

		\brief The interface describes a functionality of a system manager.
		Main tasks are registration, activation and processing existing systems.
	*/
	
	class ISystemManager: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes a ISystemManager's instance

				\param[in, out] pEventManager A pointer to IEventManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IEventManager* pEventManager) = 0;

			/*!
				\brief The method registers specified system

				\param[in] A pointer to ISystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/
			
			TDE2_API virtual E_RESULT_CODE RegisterSystem(ISystem* pSystem) = 0;

			/*!
				\brief The method unregisters specified system, but doesn't free its memory

				\param[in] A pointer to ISystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE UnregisterSystem(ISystem* pSystem) = 0;

			/*!
				\brief The method unregisters specified system and free its memory

				\param[in] A pointer to ISystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE UnregisterSystemImmediately(ISystem* pSystem) = 0;

			/*!
				\brief The method marks specified system as an active

				\param[in] A pointer to ISystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ActivateSystem(ISystem* pSystem) = 0;

			/*!
				\brief The method deactivates specified system

				\param[in] A pointer to ISystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE DeactivateSystem(ISystem* pSystem) = 0;

			/*!
				\brief The main method that should be implemented in all derived classes.
				It contains all the logic that the system will execute during engine's work.

				\param[in] pWorld A pointer to a main scene's object
				\param[in] dt A delta time's value
			*/

			TDE2_API virtual void Update(IWorld* pWorld, F32 dt) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ISystemManager)
	};
}