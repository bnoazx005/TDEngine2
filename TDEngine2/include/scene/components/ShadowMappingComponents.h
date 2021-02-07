/*!
	\file ShadowMappingComponents.h
	\date 04.05.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CShadowCasterComponent's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CShadowCasterComponent's implementation
	*/

	TDE2_API IComponent* CreateShadowCasterComponent(E_RESULT_CODE& result);


	/*!
		class CShadowCasterComponent

		\brief The class represents a component that marks the object as a shadows caster
	*/

	class CShadowCasterComponent : public CBaseComponent
	{
		public:
			friend TDE2_API IComponent* CreateShadowCasterComponent(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CShadowCasterComponent)

			/*!
				\brief The method initializes an internal state of a quad sprite

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init();

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
				\brief The method marks the object as transparent one

				\param[in] value If the value is true, then the caster is supposed as a transparent object
			*/

			TDE2_API void SetTransparentFlag(bool value);

			/*!
				\brief The method determines whether the caster is transparent object or not

				\return The method determines whether the caster is transparent object or not
			*/

			TDE2_API bool IsTransparentObject() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CShadowCasterComponent)
		protected:
			bool  mIsTransparent;
	};


	/*!
		struct TShadowCasterComponentParameters

		\brief The structure contains parameters for creation of CShadowCasterComponent
	*/

	typedef struct TShadowCasterComponentParameters : public TBaseComponentParameters
	{
		bool mIsTransparent = false;
	} TShadowCasterComponentParameters;


	/*!
		\brief A factory function for creation objects of CShadowCasterComponentFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CShadowCasterComponentFactory's implementation
	*/

	TDE2_API IComponentFactory* CreateShadowCasterComponentFactory(E_RESULT_CODE& result);


	/*!
		class CShadowCasterComponentFactory

		\brief The class is factory facility to create a new objects of CShadowCasterComponent type
	*/

	class CShadowCasterComponentFactory : public IComponentFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateShadowCasterComponentFactory(E_RESULT_CODE& result);
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CShadowCasterComponentFactory)
	};


	TDE2_DECLARE_FLAG_COMPONENT(ShadowReceiverComponent);
	TDE2_DECLARE_FLAG_COMPONENT(SkyboxComponent); // \todo Move it to another file later
}