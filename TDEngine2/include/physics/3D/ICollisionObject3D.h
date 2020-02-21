/*!
	\file ICollisionObject3D.h
	\date 20.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "./../ICollisionObject.h"
#include "./../../ecs/IComponentFactory.h"


class btCollisionShape;


namespace TDEngine2
{
	class ICollisionObjects3DVisitor;


	/*!
		interface ICollisionObject3D

		\brief The interface represents a functionality of a 3D collision object that combines both collider and rigidbody's definition
	*/

	class ICollisionObject3D : public ICollisionObject
	{
		public:
			/*!
				\brief The method initializes an internal state of a component

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			/*!
				\brief The method returns a pointer to internal representation of a collision
				shape that is used by Bullet3 physics engine

				\param[in] pVisitor A pointer to implementation of ICollisionObjects3DVisitor

				\return The method returns a pointer to internal representation of a collision
				shape that is used by Bullet3 physics engine
			*/

			TDE2_API virtual btCollisionShape* GetCollisionShape(const ICollisionObjects3DVisitor* pVisitor) const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ICollisionObject3D)
	};


	/*!
		interface ICollisionObject3DFactory

		\brief The interface represents a functionality of a factory of ICollisionObject3D objects
	*/

	class ICollisionObject3DFactory : public IComponentFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ICollisionObject3DFactory)
	};
}
