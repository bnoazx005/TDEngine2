/*!
	\file ICollisionObject.h
	\date 20.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include "./../math/TVector3.h"
#include "./../math/TQuaternion.h"


namespace TDEngine2
{
	/*!
		\brief The class contains all allowed types which a collision object can have
	*/

	ENUM_META(SECTION=physics)
	enum class E_COLLISION_OBJECT_TYPE : U16
	{
		COT_STATIC,
		COT_KINEMATIC,
		COT_DYNAMIC
	};


	/*!
		interface ICollisionObject

		\brief The interface represents a functionality of a default collision object that combines both collider and rigidbody's definition
	*/

	class ICollisionObject
	{
		public:
			/*!
				\brief The method resets all values of this object
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
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ICollisionObject)
	};
}
