/*!
	\file CBoxCollisionObject3D.h
	\date 20.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseCollisionObject3D.h"
#include <math/TVector3.h>


namespace TDEngine2
{
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

	class CBoxCollisionObject3D : public CBaseCollisionObject3D, public CPoolMemoryAllocPolicy<CBoxCollisionObject3D, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateBoxCollisionObject3D(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CBoxCollisionObject3D)

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
				\brief The method returns a pointer to internal representation of a collision
				shape that is used by Bullet3 physics engine

				\param[in] pVisitor A pointer to implementation of ICollisionObjects3DVisitor

				\return The method returns a pointer to internal representation of a collision
				shape that is used by Bullet3 physics engine
			*/

			TDE2_API btCollisionShape* GetCollisionShape(const ICollisionObjects3DVisitor* pVisitor) const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBoxCollisionObject3D)
		protected:
			TVector3 mExtents;
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