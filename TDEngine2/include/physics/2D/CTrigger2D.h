/*!
	\file CTrigger2D.h
	\date 18.11.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "./../../core/CBaseObject.h"
#include "./../../ecs/CBaseComponent.h"
#include "ITrigger2D.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CTrigger2D's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CTrigger2D's implementation
	*/

	TDE2_API IComponent* CreateTrigger2D(E_RESULT_CODE& result);


	/*!
		class CTrigger2D

		\brief The class implements a functionality of a 2D trigger
	*/

	class CTrigger2D: public ITrigger2D, public CBaseComponent
	{
		public:
			friend TDE2_API IComponent* CreateTrigger2D(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(CTrigger2D)

			/*!
				\brief The method initializes an internal state of a component

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTrigger2D)
	};


	/*!
		\brief A factory function for creation objects of CTrigger2D's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CTrigger2D's implementation
	*/

	TDE2_API IComponentFactory* CreateTrigger2DFactory(E_RESULT_CODE& result);


	/*!
		class CTrigger2DFactory

		\brief The class implements a functionality of a factory of CTrigger2D objects
	*/

	class CTrigger2DFactory : public ITrigger2DFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateTrigger2DFactory(E_RESULT_CODE& result);
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTrigger2DFactory)
	};
}
