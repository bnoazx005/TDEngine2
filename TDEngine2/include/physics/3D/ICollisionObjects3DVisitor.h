/*!
	\file ICollisionObjects3DVisitor.h
	\date 21.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "./../../utils/Types.h"
#include "./../../utils/Utils.h"


class btBoxShape;
class btSphereShape;


namespace TDEngine2
{
	class CBoxCollisionObject3D;
	class CSphereCollisionObject3D;


	/*!
		interface ICollisionObjects3DVisitor

		\brief The interface represents a functionality of a visitor
		that processes objects of ICollisionObject3D type
	*/

	class ICollisionObjects3DVisitor
	{
		public:
			/*!
				\brief The method returns a new created collision shape which is a box collider

				\param[in] box A reference to a box collision object

				\return The method returns a new created collision shape of a box collider
			*/

			TDE2_API virtual btBoxShape* CreateBoxCollisionShape(const CBoxCollisionObject3D& box) const = 0;
			
			/*!
				\brief The method returns a new created collision shape which is a sphere collider

				\param[in] sphere A reference to a sphere collision object

				\return The method returns a new created collision shape of a sphere collider
			*/

			TDE2_API virtual btSphereShape* CreateSphereCollisionShape(const CSphereCollisionObject3D& sphere) const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ICollisionObjects3DVisitor)
	};
}