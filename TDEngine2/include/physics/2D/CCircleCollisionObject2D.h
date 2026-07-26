/*!
	\file CCircleCollisionObject2D.h
	\date 09.02.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"


namespace TDEngine2
{
	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TCircleCollisionObject2DComponentData
	{
		FIELD_META(name = radius) F32  mRadius = 1.0f;

		TDE2_DECLARE_COMPONENT_META(TCircleCollisionObject2DComponentData);
	};


	/*!
		\brief A factory function for creation objects of CCircleCollisionObject2D's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CCircleCollisionObject2D's implementation
	*/

	TDE2_API IComponent* CreateCircleCollisionObject2D(E_RESULT_CODE& result);


	/*!
		class CCircleCollisionObject2D

		\brief The interface describes a functionality of a 2d circle collision object
		which is controlled by Circle2D physics engine
	*/

	class CCircleCollisionObject2D : public CBaseComponentT<CCircleCollisionObject2D, TCircleCollisionObject2DComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateCircleCollisionObject2D(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CCircleCollisionObject2D)
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CCircleCollisionObject2D)
		protected:
			F32 mRadius;
	};


	/*!
		struct TCircleCollisionObject2DParameters

		\brief The structure contains parameters for creation of CCircleCollisionObject2D
	*/

	typedef struct TCircleCollisionObject2DParameters : public TBaseComponentParameters
	{
		F32 mRadius = 1.0f;
	} TCircleCollisionObject2DParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(CircleCollisionObject2D, TCircleCollisionObject2DParameters);
}