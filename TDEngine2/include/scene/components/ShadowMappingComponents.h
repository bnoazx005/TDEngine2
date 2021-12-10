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

			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API const std::string& GetTypeName() const override;
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


	TDE2_DECLARE_COMPONENT_FACTORY(ShadowCasterComponent, TShadowCasterComponentParameters);


	TDE2_DECLARE_FLAG_COMPONENT(ShadowReceiverComponent);
	TDE2_DECLARE_FLAG_COMPONENT(SkyboxComponent); // \todo Move it to another file later
}