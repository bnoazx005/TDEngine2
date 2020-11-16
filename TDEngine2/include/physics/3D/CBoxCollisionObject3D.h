/*!
	\file CBoxCollisionObject3D.h
	\date 20.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseCollisionObject3D.h"
#include "./../../math/TVector3.h"


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

	class CBoxCollisionObject3D : public CBaseCollisionObject3D
	{
		public:
			friend TDE2_API IComponent* CreateBoxCollisionObject3D(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(CBoxCollisionObject3D)

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
		TVector3 mExtents;
	} TBoxCollisionObject3DParameters;


	/*!
		\brief A factory function for creation objects of CBoxCollisionObject3DFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBoxCollisionObject3DFactory's implementation
	*/

	TDE2_API IComponentFactory* CreateBoxCollisionObject3DFactory(E_RESULT_CODE& result);


	/*!
		class CBoxCollisionObject3DFactory

		\brief The class is factory facility to create a new objects of CBoxCollisionObject3D type
	*/

	class CBoxCollisionObject3DFactory : public ICollisionObject3DFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateBoxCollisionObject3DFactory(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* Create(const TBaseComponentParameters* pParams) const override;

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* CreateDefault(const TBaseComponentParameters& params) const override;

			/*!
				\brief The method returns an identifier of a component's type, which
				the factory serves

				\return The method returns an identifier of a component's type, which
				the factory serves
			*/

			TDE2_API TypeId GetComponentTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBoxCollisionObject3DFactory)
	};
}