/*!
	\file CSphereCollisionObject3D.h
	\date 22.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseCollisionObject3D.h"
#include "../../math/TVector3.h"


namespace TDEngine2
{
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

	class CSphereCollisionObject3D : public CBaseCollisionObject3D, public CPoolMemoryAllocPolicy<CSphereCollisionObject3D, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateSphereCollisionObject3D(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CSphereCollisionObject3D)

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

				\param[in] radius A radius of a sphere collider
			*/

			TDE2_API virtual void SetRadius(F32 radius);

			/*!
				\brief The method returns a radius of a sphere collider

				\return The method returns a radius of a sphere collider
			*/

			TDE2_API virtual F32 GetRadius() const;

			/*!
				\brief The method returns a pointer to internal representation of a collision
				shape that is used by Bullet3 physics engine

				\param[in] pVisitor A pointer to implementation of ICollisionObjects3DVisitor

				\return The method returns a pointer to internal representation of a collision
				shape that is used by Bullet3 physics engine
			*/

			TDE2_API btCollisionShape* GetCollisionShape(const ICollisionObjects3DVisitor* pVisitor) const override;
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