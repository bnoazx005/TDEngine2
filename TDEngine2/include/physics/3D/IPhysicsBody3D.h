/*!
	\file IPhysicsBody3D.h
	\date 01.08.2026
	\authors Kasimov Ildar
*/

#pragma once


#include "../../utils/Types.h"
#include "../../physics/ICollisionObject.h"
#include "../../ecs/IComponent.h"


namespace TDEngine2
{
	/*!
		interface IPhysicsBody3D

		\brief The interface describes a type that defines 3D physical object
	*/

	class IPhysicsBody3D : public virtual IComponent
	{
		public:
			TDE2_REGISTER_COMPONENT_TYPE(IPhysicsBody3D)

			/*!
				\brief The method specifies a type of a 2d collision object

				\param[in] type A type of 2d collision object
			*/

			TDE2_API virtual void SetCollisionType(E_COLLISION_OBJECT_TYPE type) = 0;

			/*!
				\brief The method specifies a mass of an object. Only positive value
				can be assigned. If you try to enter negative one the mass will be set
				into 1.0f

				\param[in] mass A mass of an object
			*/

			TDE2_API virtual void SetMass(F32 mass) = 0;

			/*!
				\brief The method returns a type of a 2d collision object

				\return The method returns a type of a 2d collision object
			*/

			TDE2_API virtual E_COLLISION_OBJECT_TYPE GetCollisionType() const = 0;

			/*!
				\brief The method returns a mass of an object

				\return The method returns a mass of an object
			*/

			TDE2_API virtual F32 GetMass() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IPhysicsBody3D)
	};
}
