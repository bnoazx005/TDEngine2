/*!
	\file CSphereCollisionObject3D.h
	\date 22.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include <ecs/CBaseComponent.h>
#include <math/TVector3.h>
#include <physics/3D/ISphereCollisionObject3D.h>


namespace TDEngine2
{
	CLASS_META(SECTION = bullet_ecs_plugin, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TSphereCollisionObject3DComponentData
	{
		FIELD_META(name = radius) F32 mRadius = 1.0f;

		TDE2_DECLARE_COMPONENT_META(TSphereCollisionObject3DComponentData);
	};


	/*!
		\brief A factory function for creation objects of CSphereCollisionObject3D's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CSphereCollisionObject3D's implementation
	*/

	TDE2_API IComponent* CreateSphereCollisionObject3D(E_RESULT_CODE& result);


	/*!
		class CSphereCollisionObject3D

		\brief The interface describes a functionality of a sphere collision object
		which is controlled by Bullet3 physics engine
	*/

	class CSphereCollisionObject3D : public CBaseComponentT<CSphereCollisionObject3D, TSphereCollisionObject3DComponentData>, public ISphereCollisionObject3D
	{
		public:
			friend TDE2_API IComponent* CreateSphereCollisionObject3D(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CSphereCollisionObject3D)

			/*!
				\brief The method sets up a radius of a collider

				\param[in] radius A radius of a sphere collider
			*/

			TDE2_API void SetRadius(F32 radius) override;

			/*!
				\brief The method returns a radius of a sphere collider

				\return The method returns a radius of a sphere collider
			*/

			TDE2_API F32 GetRadius() const override;
			
			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API const std::string& GetTypeName() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSphereCollisionObject3D)
		protected:
			F32 mRadius;
	};


	/*!
		struct TSphereCollisionObject3DParameters

		\brief The structure contains parameters for creation of CSphereCollisionObject3D
	*/

	typedef struct TSphereCollisionObject3DParameters : public TBaseComponentParameters
	{
		F32 mRadius = 1.0f;
	} TSphereCollisionObject3DParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(SphereCollisionObject3D, TSphereCollisionObject3DParameters);
}