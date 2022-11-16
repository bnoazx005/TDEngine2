/*!
	\file IBoxCollisionObject3D.h
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
		interface IBoxCollisionObject3D

		\brief The interface describes a functionality of a 3d box collision object
	*/

	class IBoxCollisionObject3D : public virtual ICollisionObject, public virtual IComponent
	{
		public:
			TDE2_REGISTER_COMPONENT_TYPE(IBoxCollisionObject3D)

			/*!
				\brief The method sets up sizes of a box collider

				\param[in] extents A vector each component defines width, height and depth of the box collider
			*/

			TDE2_API virtual void SetSizes(const TVector3& extents) = 0;

			/*!
				\brief The method returns sizes of a box collider

				\return The method returns sizes of a box collider
			*/

			TDE2_API virtual const TVector3& GetSizes() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IBoxCollisionObject3D)
	};
}
