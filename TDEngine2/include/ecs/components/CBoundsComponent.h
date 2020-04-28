/*!
	\file CBoundsComponent.h
	\date 28.04.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include "../../math/TAABB.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CBoundsComponent's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBoundsComponent's implementation
	*/

	TDE2_API IComponent* CreateBoundsComponent(E_RESULT_CODE& result);


	/*!
		class CBoundsComponent

		\brief The class represents a component that stores information about object's boundaries
	*/

	class CBoundsComponent: public CBaseComponent
	{
		public:
			friend TDE2_API IComponent* CreateBoundsComponent(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CBoundsComponent)

			/*!
				\brief The method initializes an internal state of a quad sprite

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init();

			/*!
				\brief The method defines boundaries for the object based on AABB

				\param[in] aabbBounds An object that holds information about boundaries
			*/

			TDE2_API void SetBounds(const TAABB& aabbBounds);

			/*!
				\brief The method changes so called 'dirty' flag of the object that means that it was modified

				\param[in] value A new state of the flag, true means that the object has been changed
			*/

			TDE2_API void SetDirty(bool value);

			/*!
				\brief The method returns a boundaries of the object

				\return The method returns AABB object that contains boundaries of the object
			*/

			TDE2_API const TAABB& GetBounds() const;

			/*!
				\brief The method returns state of the object

				\return The method returns true if the object has been changed, but not been updated yet
			*/

			TDE2_API bool IsDirty() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBoundsComponent)
		protected:
			TAABB mBounds;

			bool  mIsDirty;
	};


	/*!
		struct TBoundsComponentParameters

		\brief The structure contains parameters for creation of CBoundsComponent
	*/

	typedef struct TBoundsComponentParameters : public TBaseComponentParameters
	{
		TAABB mBounds;
	} TBoundsComponentParameters;


	/*!
		\brief A factory function for creation objects of CBoundsComponentFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBoundsComponentFactory's implementation
	*/

	TDE2_API IComponentFactory* CreateBoundsComponentFactory(E_RESULT_CODE& result);


	/*!
		class CBoundsComponentFactory

		\brief The class is factory facility to create a new objects of CBoundsComponent type
	*/

	class CBoundsComponentFactory : public IComponentFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateBoundsComponentFactory(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init();

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBoundsComponentFactory)
	};
}