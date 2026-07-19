/*!
	\file CDirectionalLight.h
	\date 05.05.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "ILight.h"
#include "../../ecs/CBaseComponent.h"


namespace TDEngine2
{
	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TDirectionalLightComponentData
	{
		FIELD_META(name = color) TColor32F mColor = TColorUtils::mWhite;
		FIELD_META(name = intensity) F32   mIntensity = 1.0f;

		TDE2_DECLARE_COMPONENT_META(TDirectionalLightComponentData);
	};


	/*!
		\brief A factory function for creation objects of CDirectionalLight's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CDirectionalLight's implementation
	*/

	TDE2_API IComponent* CreateDirectionalLight(E_RESULT_CODE& result);


	/*!
		class CDirectionalLight

		\brief The class represents directed light source which is an analogue of sun light 
	*/

	class CDirectionalLight : public CBaseComponentT<CDirectionalLight, TDirectionalLightComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateDirectionalLight(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CDirectionalLight)

			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API const std::string& GetTypeName() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CDirectionalLight)
	};


	TDE2_DECLARE_COMPONENT_FACTORY(DirectionalLight, TDirectionalLightParameters);
}
