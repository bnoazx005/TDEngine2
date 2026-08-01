/*!
	\file CPhysicsBody3D.h
	\date 26.07.2026
	\authors Kasimov Ildar
*/

#pragma once


#include <core/CBaseObject.h>
#include <ecs/CBaseComponent.h>
#include <physics/ICollisionObject.h>
#include <physics/3D/IPhysicsBody3D.h>


namespace TDEngine2
{
	CLASS_META(SECTION = bullet_ecs_plugin, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TPhysicsBody3DComponentData
	{
		FIELD_META(name = type) E_COLLISION_OBJECT_TYPE mType = E_COLLISION_OBJECT_TYPE::COT_DYNAMIC;

		FIELD_META(name = mass) F32                     mMass = 1.0f;

		bool                                            mHasChanged = false;

		TDE2_DECLARE_COMPONENT_META(TPhysicsBody3DComponentData);
	};


	/*!
		\brief A factory function for creation objects of CPhysicsBody3D's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CPhysicsBody3D's implementation
	*/

	TDE2_API IComponent* CreatePhysicsBody3D(E_RESULT_CODE& result);


	/*!
		class CPhysicsBody3D

		\brief The class implements a functionality of a 3D physics object that has mass and specifies a type of physics interaction
	*/

	class CPhysicsBody3D : public CBaseComponentT<CPhysicsBody3D, TPhysicsBody3DComponentData>, public IPhysicsBody3D
	{
		public:
			friend TDE2_API IComponent* CreatePhysicsBody3D(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(IPhysicsBody3D)

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

			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API const std::string& GetTypeName() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPhysicsBody3D)
	};


	/*!
		struct TPhysicsBody3DParameters

		\brief The structure contains parameters for creation of CPhysicsBody3D
	*/

	typedef struct TPhysicsBody3DParameters : public TBaseComponentParameters
	{
	} TPhysicsBody3DParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(PhysicsBody3D, TPhysicsBody3DParameters);
}
