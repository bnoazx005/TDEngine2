/*!
	\file ShadowMappingComponents.h
	\date 04.05.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"


namespace TDEngine2
{
	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TShadowCasterComponentData
	{
		FIELD_META(name = transparent) bool mIsTransparent = false;

		TDE2_DECLARE_COMPONENT_META(TShadowCasterComponentData);
	};


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

	class CShadowCasterComponent : public CBaseComponentT<CShadowCasterComponent, TShadowCasterComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateShadowCasterComponent(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CShadowCasterComponent)

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