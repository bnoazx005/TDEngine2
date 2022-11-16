/*!
	\file ICapsuleCollisionObject3D.h
	\date 16.11.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "../../utils/Types.h"
#include "../../physics/ICollisionObject.h"
#include "../../ecs/IComponent.h"


namespace TDEngine2
{
	/*!
		interface ICapsuleCollisionObject3D

		\brief The interface describes a functionality of a Capsule collision object
	*/

	class ICapsuleCollisionObject3D : public virtual ICollisionObject, public virtual IComponent
	{
		public:
			TDE2_REGISTER_COMPONENT_TYPE(ICapsuleCollisionObject3D)

			/*!
				\brief The method sets up a radius of a collider

				\param[in] radius A radius of a Capsule collider
			*/

			TDE2_API virtual E_RESULT_CODE SetRadius(F32 radius) = 0;

			TDE2_API virtual E_RESULT_CODE SetHeight(F32 height) = 0;

			/*!
				\brief The method returns a radius of a Capsule collider

				\return The method returns a radius of a Capsule collider
			*/

			TDE2_API virtual F32 GetRadius() const = 0;

			/*!
				\brief The method returns a height of a Capsule collider
			*/

			TDE2_API virtual F32 GetHeight() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ICapsuleCollisionObject3D)
	};
}
