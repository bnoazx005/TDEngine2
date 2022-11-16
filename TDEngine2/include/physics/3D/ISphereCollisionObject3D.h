/*!
	\file ISphereCollisionObject3D.h
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
		interface ISphereCollisionObject3D

		\brief The interface describes a functionality of a sphere collision object
	*/

	class ISphereCollisionObject3D : public virtual ICollisionObject, public virtual IComponent
	{
		public:
			TDE2_REGISTER_COMPONENT_TYPE(ISphereCollisionObject3D)

			/*!
				\brief The method sets up a radius of a collider

				\param[in] radius A radius of a sphere collider
			*/

			TDE2_API virtual void SetRadius(F32 radius) = 0;

			/*!
				\brief The method returns a radius of a sphere collider

				\return The method returns a radius of a sphere collider
			*/

			TDE2_API virtual F32 GetRadius() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ISphereCollisionObject3D)
	};
}
