/*!
	\file CCapsuleCollisionObject3D.h
	\date 29.10.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseCollisionObject3D.h"
#include "../../math/TVector3.h"


namespace TDEngine2
{
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

	class CCapsuleCollisionObject3D : public CBaseCollisionObject3D, public CPoolMemoryAllocPolicy<CCapsuleCollisionObject3D, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateCapsuleCollisionObject3D(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CCapsuleCollisionObject3D)

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
				\brief The method creates a new deep copy of the instance and returns a smart pointer to it.
				The original state of the object stays the same

				\param[in] pDestObject A valid pointer to an object which the properties will be assigned into
			*/

			TDE2_API E_RESULT_CODE Clone(IComponent*& pDestObject) const override;

			/*!
				\brief The method sets up a radius of a collider

				\param[in] radius A radius of a Capsule collider
			*/

			TDE2_API virtual E_RESULT_CODE SetRadius(F32 radius);

			TDE2_API virtual E_RESULT_CODE SetHeight(F32 height);

			/*!
				\brief The method returns a radius of a Capsule collider

				\return The method returns a radius of a Capsule collider
			*/

			TDE2_API virtual F32 GetRadius() const;

			/*!
				\brief The method returns a height of a Capsule collider
			*/

			TDE2_API virtual F32 GetHeight() const;

			/*!
				\brief The method returns a pointer to internal representation of a collision
				shape that is used by Bullet3 physics engine

				\param[in] pVisitor A pointer to implementation of ICollisionObjects3DVisitor

				\return The method returns a pointer to internal representation of a collision
				shape that is used by Bullet3 physics engine
			*/

			TDE2_API btCollisionShape* GetCollisionShape(const ICollisionObjects3DVisitor* pVisitor) const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CCapsuleCollisionObject3D)
		protected:
			F32 mRadius;
			F32 mHeight;
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