/*!
	\file CCircleCollisionObject2D.h
	\date 09.02.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseCollisionObject2D.h"


namespace TDEngine2
{
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

	class CCircleCollisionObject2D : public CBaseCollisionObject2D
	{
		public:
			friend TDE2_API IComponent* CreateCircleCollisionObject2D(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CCircleCollisionObject2D)

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;

			/*!
				\brief The method serializes object's state into given stream

				\param[in, out] pWriter An output stream of data that writes information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Save(IArchiveWriter* pWriter) override;

			/*!
				\brief The method sets up a radius of a collider

				\param[in] radius A radius of a circle collider
			*/

			TDE2_API virtual void SetRadius(F32 radius);
			
			/*!
				\brief The method returns a radius of a circle collider

				\return The method returns a radius of a circle collider
			*/

			TDE2_API virtual F32 GetRadius() const;

			/*!
				\brief The method returns a rvalue reference to internal representation of a collision
				shape that is used by Box2D physics engine

				\param[in] pVisitor A pointer to implementation of ICollisionObjectsVisitor

				\param[in] callback A callback that is called when a new definition of a shape was created
			*/

			TDE2_API void GetCollisionShape(const ICollisionObjectsVisitor* pVisitor, const std::function<void(const b2Shape*)>& callback) const override;
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