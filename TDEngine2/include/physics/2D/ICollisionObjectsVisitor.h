/*!
	\file ICollisionObjectsVisitor.h
	\date 11.02.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "./../../utils/Types.h"
#include "./../../utils/Utils.h"
#include "Box2D.h"


namespace TDEngine2
{
	class CBoxCollisionObject2D;
	class CCircleCollisionObject2D;


	/*!
		interface ICollisionObjectsVisitor

		\brief The interface represents a functionality of a visitor
		that processes objects of ICollisionObject2D type
	*/

	class ICollisionObjectsVisitor
	{
		public:
			/*!
				\brief The method returns a new created collision shape which is a box collider
				
				\param[in] box A reference to a box collision object

				\return The method returns a new created collision shape of a box collider
			*/

			TDE2_API virtual b2PolygonShape CreateBoxCollisionShape(const CBoxCollisionObject2D& box) const = 0;

			/*!
				\brief The method returns a new created collision shape which is a circle collider
				
				\param[in] circle A reference to a circle collision object

				\return The method returns a new created collision shape of a circle collider
			*/

			TDE2_API virtual b2CircleShape CreateCircleCollisionShape(const CCircleCollisionObject2D& circle) const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ICollisionObjectsVisitor)
	};
}