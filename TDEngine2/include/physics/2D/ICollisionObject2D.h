/*!
	\file ICollisionObject2D.h
	\date 08.02.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "./../../utils/Types.h"
#include "./../../utils/Utils.h"
#include "./../../math/TVector3.h"
#include "./../../math/TQuaternion.h"
#include "./../../ecs/IComponentFactory.h"
#include <functional>
#include "Box2D.h"


namespace TDEngine2
{
	class ICollisionObjectsVisitor;


	/*!
		\brief The class contains all allowed types which a 2d collision object can have
	*/

	enum class E_COLLISION_OBJECT_TYPE : U16
	{
		COT_STATIC,
		COT_KINEMATIC,
		COT_DYNAMIC
	};


	/*!
		interface ICollisionObject2D

		\brief The interface represents a functionality of a
		2D collision object that combines both collider and rigidbody's definition
	*/

	class ICollisionObject2D
	{
		public:
			/*!
				\brief The method initializes an internal state of a component

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			/*!
				\brief The method resets all values of this transform
			*/

			TDE2_API virtual void Reset() = 0;
			
			/*!
				\brief The method returns true if a state of a component was changed

				\return The method returns true if a state of a component was changed
			*/

			TDE2_API virtual bool HasChanged() const = 0;

			/*!
				\brief The method specifies a type of a 2d collision object

				\param[in] type A type of 2d collision object 
			*/

			TDE2_API virtual void SetCollisionType(E_COLLISION_OBJECT_TYPE type) = 0;

			/*!
				\brief The method specifies a mass of an object

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
