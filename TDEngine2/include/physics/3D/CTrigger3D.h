/*!
	\file CTrigger3D.h
	\date 07.12.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../../core/CBaseObject.h"
#include "../../ecs/CBaseComponent.h"
#include "ITrigger3D.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CTrigger3D's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CTrigger3D's implementation
	*/

	TDE2_API IComponent* CreateTrigger3D(E_RESULT_CODE& result);


	/*!
		class CTrigger3D

		\brief The class implements a functionality of a 3D trigger
	*/

	class CTrigger3D : public ITrigger3D, public CBaseComponent
	{
		public:
			friend TDE2_API IComponent* CreateTrigger3D(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CTrigger3D)

			/*!
				\brief The method initializes an internal state of a component

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTrigger3D)
	};


	/*!
		\brief A factory function for creation objects of CTrigger3D's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CTrigger3D's implementation
	*/

	TDE2_API IComponentFactory* CreateTrigger3DFactory(E_RESULT_CODE& result);


	/*!
		class CTrigger3DFactory

		\brief The class implements a functionality of a factory of CTrigger3D objects
	*/

	class CTrigger3DFactory : public ITrigger3DFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateTrigger3DFactory(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTrigger3DFactory)
	};
}
