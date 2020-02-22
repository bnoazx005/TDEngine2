/*!
	\file CSphereCollisionObject3D.h
	\date 22.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseCollisionObject3D.h"
#include "./../../math/TVector3.h"


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

	class CSphereCollisionObject3D : public CBaseCollisionObject3D
	{
		public:
			friend TDE2_API IComponent* CreateSphereCollisionObject3D(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(CSphereCollisionObject3D)

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
		F32 mRadius;
	} TSphereCollisionObject3DParameters;


	/*!
		\brief A factory function for creation objects of CSphereCollisionObject3DFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CSphereCollisionObject3DFactory's implementation
	*/

	TDE2_API IComponentFactory* CreateSphereCollisionObject3DFactory(E_RESULT_CODE& result);


	/*!
		class CSphereCollisionObject3DFactory

		\brief The class is factory facility to create a new objects of CSphereCollisionObject3D type
	*/

	class CSphereCollisionObject3DFactory : public ICollisionObject3DFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateSphereCollisionObject3DFactory(E_RESULT_CODE& result);
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSphereCollisionObject3DFactory)
	};
}