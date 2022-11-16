/*!
	\file IConvexHullCollisionObject3D.h
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
		interface IConvexHullCollisionObject3D

		\brief The interface describes a functionality of a ConvexHull collision object
	*/

	class IConvexHullCollisionObject3D : public virtual ICollisionObject, public virtual IComponent
	{
		public:
			TDE2_REGISTER_COMPONENT_TYPE(IConvexHullCollisionObject3D)


			TDE2_API virtual E_RESULT_CODE SetVertices(const std::vector<TVector4>& vertices) = 0;

			TDE2_API virtual const std::vector<TVector4>& GetVertices() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IConvexHullCollisionObject3D)
	};
}
