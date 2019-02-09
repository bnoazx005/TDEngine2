/*!
	\file CBaseCollisionObject2D.h
	\date 08.02.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "ICollisionObject2D.h"
#include "./../../ecs/CBaseComponent.h"
#include "./../../ecs/IComponentFactory.h"
#include "./../../math/TMatrix4.h"


namespace TDEngine2
{
	/*!
		class CBaseCollisionObject2D

		\brief The class is a base class for all 2D collidable objects
		that are used in the engine
	*/

	class CBaseCollisionObject2D : public ICollisionObject2D, public CBaseComponent
	{
		public:
			TDE2_REGISTER_TYPE(CBaseCollisionObject2D)

			/*!
				\brief The method initializes an internal state of a component
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method resets all values of this transform
			*/

			TDE2_API void Reset() override;

			/*!
				\brief The method returns true if a state of a component was changed

				\return The method returns true if a state of a component was changed
			*/

			TDE2_API bool HasChanged() const override;

			/*!
				\brief The method specifies a type of a 2d collision object

				\param[in] type A type of 2d collision object
			*/

			TDE2_API void SetCollisionType(E_COLLISION_OBJECT_TYPE type) override;
			
			/*!
				\brief The method specifies a mass of an object. Only positive value
				can be assigned. If you try to enter negative one the mass will be set
				into 1.0f

				\param[in] mass A mass of an object
			*/

			TDE2_API void SetMass(F32 mass) override;

			/*!
				\brief The method returns a type of a 2d collision object

				\return The method returns a type of a 2d collision object
			*/

			TDE2_API E_COLLISION_OBJECT_TYPE GetCollisionType() const override;

			/*!
				\brief The method returns a mass of an object

				\return The method returns a mass of an object
			*/

			TDE2_API F32 GetMass() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseCollisionObject2D)
		protected:
			E_COLLISION_OBJECT_TYPE mType;

			F32                     mMass;

			bool                    mHasChanged;
	};
}
