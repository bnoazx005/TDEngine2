/*!
	\file CBoxCollisionObject3D.h
	\date 20.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include <ecs/CBaseComponent.h>
#include <math/TVector3.h>
#include <physics/3D/IBoxCollisionObject3D.h>


namespace TDEngine2
{
	CLASS_META(SECTION = bullet_ecs_plugin, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TBoxCollisionObject3DComponentData
	{
		FIELD_META(name = extents) TVector3 mExtents = TVector3{ 1.0f };

		TDE2_DECLARE_COMPONENT_META(TBoxCollisionObject3DComponentData);
	};


	/*!
		\brief A factory function for creation objects of CBoxCollisionObject3D's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBoxCollisionObject3D's implementation
	*/

	TDE2_API IComponent* CreateBoxCollisionObject3D(E_RESULT_CODE& result);


	/*!
		class CBoxCollisionObject3D

		\brief The interface describes a functionality of a 3d box collision object
		which is controlled by Bullet3 physics engine
	*/

	class CBoxCollisionObject3D : public CBaseComponentT<CBoxCollisionObject3D, TBoxCollisionObject3DComponentData>, public IBoxCollisionObject3D
	{
		public:
			friend TDE2_API IComponent* CreateBoxCollisionObject3D(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(IBoxCollisionObject3D)

			/*!
				\brief The method sets up sizes of a box collider

				\param[in] extents A vector each component defines width, height and depth of the box collider
			*/

			TDE2_API virtual void SetSizes(const TVector3& extents);

			/*!
				\brief The method returns sizes of a box collider

				\return The method returns sizes of a box collider
			*/

			TDE2_API virtual const TVector3& GetSizes() const;

			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API const std::string& GetTypeName() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBoxCollisionObject3D)
	};


	/*!
		struct TBoxCollisionObject3DParameters

		\brief The structure contains parameters for creation of CBoxCollisionObject3D
	*/

	typedef struct TBoxCollisionObject3DParameters : public TBaseComponentParameters
	{
		TVector3 mExtents = TVector3(1.0f);
	} TBoxCollisionObject3DParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(BoxCollisionObject3D, TBoxCollisionObject3DParameters);
}