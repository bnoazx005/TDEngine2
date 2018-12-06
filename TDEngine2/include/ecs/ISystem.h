/*!
	\file ISystem.h
	\date 29.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/IBaseObject.h"
#include "./../utils/Utils.h"


namespace TDEngine2
{
	class IWorld;

	/*!
		interface ISystem

		\brief The interface describes a functionality of a system.
	*/

	class ISystem : public virtual IBaseObject
	{
		public:
			/*!
				\brief The method inject components array into a system
				
				\param[in] pWorld A pointer to a main scene's object
			*/

			TDE2_API virtual void InjectBindings(IWorld* pWorld) = 0;

			/*!
				\brief The main method that should be implemented in all derived classes.
				It contains all the logic that the system will execute during engine's work.

				\param[in] pWorld A pointer to a main scene's object
				\param[in] dt A delta time's value
			*/

			TDE2_API virtual void Update(IWorld* pWorld, F32 dt) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ISystem)
	};
}