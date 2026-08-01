/*!
	\file CCapsuleCollisionObject3D.h
	\date 29.10.2022
	\authors Kasimov Ildar
*/

#pragma once


#include <ecs/CBaseComponent.h>
#include <math/TVector3.h>
#include <physics/3D/ICapsuleCollisionObject3D.h>


namespace TDEngine2
{
	CLASS_META(SECTION = bullet_ecs_plugin, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TCapsuleCollisionObject3DComponentData
	{
		FIELD_META(name = radius) F32 mRadius = 1.0f;
		FIELD_META(name = height) F32 mHeight = 1.0f;

		TDE2_DECLARE_COMPONENT_META(TCapsuleCollisionObject3DComponentData);
	};


	/*!
		\brief A factory function for creation objects of CCapsuleCollisionObject3D's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CCapsuleCollisionObject3D's implementation
	*/

	TDE2_API IComponent* CreateCapsuleCollisionObject3D(E_RESULT_CODE& result);


	/*!
		class CCapsuleCollisionObject3D

		\brief The interface describes a functionality of a capsule collision object
		which is controlled by Bullet3 physics engine
	*/

	class CCapsuleCollisionObject3D : public CBaseComponentT<CCapsuleCollisionObject3D, TCapsuleCollisionObject3DComponentData>, public ICapsuleCollisionObject3D
	{
		public:
			friend TDE2_API IComponent* CreateCapsuleCollisionObject3D(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CCapsuleCollisionObject3D)

			/*!
				\brief The method sets up a radius of a collider

				\param[in] radius A radius of a Capsule collider
			*/

			TDE2_API E_RESULT_CODE SetRadius(F32 radius) override;

			TDE2_API E_RESULT_CODE SetHeight(F32 height) override;

			/*!
				\brief The method returns a radius of a Capsule collider

				\return The method returns a radius of a Capsule collider
			*/

			TDE2_API F32 GetRadius() const override;

			/*!
				\brief The method returns a height of a Capsule collider
			*/

			TDE2_API F32 GetHeight() const override;

			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API const std::string& GetTypeName() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CCapsuleCollisionObject3D)
	};


	/*!
		struct TCapsuleCollisionObject3DParameters

		\brief The structure contains parameters for creation of CCapsuleCollisionObject3D
	*/

	typedef struct TCapsuleCollisionObject3DParameters : public TBaseComponentParameters
	{
		F32 mRadius = 1.0f;
		F32 mHeight = 1.0f;
	} TCapsuleCollisionObject3DParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(CapsuleCollisionObject3D, TCapsuleCollisionObject3DParameters);
}