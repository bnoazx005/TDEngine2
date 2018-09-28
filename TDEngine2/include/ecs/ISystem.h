/*!
	\file ISystem.h
	\date 29.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/IBaseObject.h"


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
				\brief The main method that should be implemented in all derived classes.
				It contains all the logic that the system will execute during engine's work.

				\param[in] pWorld A pointer to a main scene's object
				\param[in] dt A delta time's value
			*/

			TDE2_API virtual void Update(IWorld* pWorld, float dt) = 0;
		protected:
			TDE2_API ISystem() = default;
			TDE2_API virtual ~ISystem() = default;
			TDE2_API ISystem(const ISystem& system) = delete;
			TDE2_API virtual ISystem& operator=(const ISystem& system) = delete;
	};
}