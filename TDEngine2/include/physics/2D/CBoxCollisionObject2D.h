/*!
	\file CBoxCollisionObject2D.h
	\date 09.02.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseCollisionObject2D.h"


namespace TDEngine2
{
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

	class CBoxCollisionObject2D : public CBaseCollisionObject2D
	{
		public:
			friend TDE2_API IComponent* CreateBoxCollisionObject2D(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(CBoxCollisionObject2D)

			/*!
				\brief The method sets up a width of a box collider

				\param[in] width A width of a box collider
			*/

			TDE2_API virtual void SetWidth(F32 width);

			/*!
				\brief The method sets up a height of a box collider

				\param[in] width A height of a box collider
			*/

			TDE2_API virtual void SetHeight(F32 height);

			/*!
				\brief The method returns a width of a box collider

				\return The method returns a width of a box collider
			*/

			TDE2_API virtual F32 GetWidth() const;

			/*!
				\brief The method returns a height of a box collider

				\return The method returns a height of a box collider
			*/

			TDE2_API virtual F32 GetHeight() const;

			/*!
				\brief The method returns a rvalue reference to internal representation of a collision
				shape that is used by Box2D physics engine

				\param[in] pVisitor A pointer to implementation of ICollisionObjectsVisitor

				\param[in] callback A callback that is called when a new definition of a shape was created
			*/

			TDE2_API void GetCollisionShape(const ICollisionObjectsVisitor* pVisitor, const std::function<void(const b2Shape*)>& callback) const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBoxCollisionObject2D)
		protected:
			F32 mWidth;

			F32 mHeight;
	};


	/*!
		struct TBoxCollisionObject2DParameters

		\brief The structure contains parameters for creation of CBoxCollisionObject2D
	*/

	typedef struct TBoxCollisionObject2DParameters : public TBaseComponentParameters
	{
	} TBoxCollisionObject2DParameters;


	/*!
		\brief A factory function for creation objects of CBoxCollisionObject2DFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBoxCollisionObject2DFactory's implementation
	*/

	TDE2_API IComponentFactory* CreateBoxCollisionObject2DFactory(E_RESULT_CODE& result);


	/*!
		class CBoxCollisionObject2DFactory

		\brief The class is factory facility to create a new objects of CBoxCollisionObject2D type
	*/

	class CBoxCollisionObject2DFactory : public ICollisionObject2DFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateBoxCollisionObject2DFactory(E_RESULT_CODE& result);
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBoxCollisionObject2DFactory)
	};
}