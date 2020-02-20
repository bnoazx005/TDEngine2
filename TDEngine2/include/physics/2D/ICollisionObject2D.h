/*!
	\file ICollisionObject2D.h
	\date 08.02.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "./../ICollisionObject.h"
#include "./../../ecs/IComponentFactory.h"
#include <functional>
#include "Box2D.h"


namespace TDEngine2
{
	class ICollisionObjectsVisitor;


	/*!
		interface ICollisionObject2D

		\brief The interface represents a functionality of a
		2D collision object that combines both collider and rigidbody's definition
	*/

	class ICollisionObject2D: public ICollisionObject
	{
		public:
			/*!
				\brief The method initializes an internal state of a component

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			/*!
				\brief The method returns a rvalue reference to internal representation of a collision 
				shape that is used by Box2D physics engine
				
				\param[in] pVisitor A pointer to implementation of ICollisionObjectsVisitor

				\param[in] callback A callback that is called when a new definition of a shape was created
			*/

			TDE2_API virtual void GetCollisionShape(const ICollisionObjectsVisitor* pVisitor, const std::function<void(const b2Shape*)>& callback) const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ICollisionObject2D)
	};


	/*!
		interface ICollisionObject2DFactory

		\brief The interface represents a functionality of a factory of ICollisionObject2D objects
	*/

	class ICollisionObject2DFactory : public IComponentFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;
		protected:
	};
}
