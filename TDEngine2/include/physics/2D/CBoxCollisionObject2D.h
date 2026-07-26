/*!
	\file CBoxCollisionObject2D.h
	\date 09.02.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"


namespace TDEngine2
{
	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TBoxCollisionObject2DComponentData
	{
		FIELD_META(name = width) F32  mWidth = 1.0f;
		FIELD_META(name = height) F32 mHeight = 1.0f;

		TDE2_DECLARE_COMPONENT_META(TBoxCollisionObject2DComponentData);
	};


	/*!
		\brief A factory function for creation objects of CBoxCollisionObject2D's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBoxCollisionObject2D's implementation
	*/

	TDE2_API IComponent* CreateBoxCollisionObject2D(E_RESULT_CODE& result);


	/*!
		class CBoxCollisionObject2D

		\brief The interface describes a functionality of a 2d box collision object
		which is controlled by Box2D physics engine
	*/

	class CBoxCollisionObject2D : public CBaseComponentT<CBoxCollisionObject2D, TBoxCollisionObject2DComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateBoxCollisionObject2D(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CBoxCollisionObject2D)
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBoxCollisionObject2D)
	};


	/*!
		struct TBoxCollisionObject2DParameters

		\brief The structure contains parameters for creation of CBoxCollisionObject2D
	*/

	typedef struct TBoxCollisionObject2DParameters : public TBaseComponentParameters
	{
		F32 mWidth = 1.0f;
		F32 mHeight = 1.0f;
	} TBoxCollisionObject2DParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(BoxCollisionObject2D, TBoxCollisionObject2DParameters);
}