/*!
	\file CPhysicsBody2D.h
	\date 21.07.2026
	\authors Kasimov Ildar
*/

#pragma once


#include "../../core/CBaseObject.h"
#include "../../ecs/CBaseComponent.h"
#include "../ICollisionObject.h"


namespace TDEngine2
{
	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TPhysicsBody2DComponentData
	{
		FIELD_META(name = type) E_COLLISION_OBJECT_TYPE mType = E_COLLISION_OBJECT_TYPE::COT_DYNAMIC;

		FIELD_META(name = mass) F32                     mMass = 1.0f;

		bool                                            mHasChanged = false;

		TDE2_DECLARE_COMPONENT_META(TPhysicsBody2DComponentData);
	};


	/*!
		\brief A factory function for creation objects of CPhysicsBody2D's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CPhysicsBody2D's implementation
	*/

	TDE2_API IComponent* CreatePhysicsBody2D(E_RESULT_CODE& result);


	/*!
		class CPhysicsBody2D

		\brief The class implements a functionality of a 2d physical object that can interact with other objects based on its mass and type
	*/

	class CPhysicsBody2D : public CBaseComponentT<CPhysicsBody2D, TPhysicsBody2DComponentData>
	{
		public:
			friend TDE2_API IComponent* CreatePhysicsBody2D(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CPhysicsBody2D)

			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API const std::string& GetTypeName() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPhysicsBody2D)
	};


	/*!
		struct TPhysicsBody2DParameters
	*/

	typedef struct TPhysicsBody2DParameters : public TBaseComponentParameters
	{
	} TPhysicsBody2DParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(PhysicsBody2D, TPhysicsBody2DParameters);
}