/*!
	\file CSpotLight.h
	\date 23.03.2024
	\authors Kasimov Ildar
*/

#pragma once


#include "ILight.h"
#include "../../ecs/CBaseComponent.h"


namespace TDEngine2
{
	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TSpotLightComponentData
	{
		FIELD_META(name = color) TColor32F mColor = TColorUtils::mWhite;
		FIELD_META(name = intensity) F32   mIntensity = 1.0f;
		FIELD_META(name = angle) F32       mConeAngle = 0.0f;
		FIELD_META(name = range) F32       mRange = 1.0f;
		TDE2_DECLARE_COMPONENT_META(TSpotLightComponentData);
	};

	/*!
		\brief A factory function for creation objects of CSpotLight's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A Spoter to CSpotLight's implementation
	*/

	TDE2_API IComponent* CreateSpotLight(E_RESULT_CODE& result);


	/*!
		class CSpotLight 
	*/

	class CSpotLight : public CBaseComponentT<CSpotLight, TSpotLightComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateSpotLight(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CSpotLight)

			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API const std::string& GetTypeName() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSpotLight)
	};


	TDE2_DECLARE_COMPONENT_FACTORY(SpotLight, TSpotLightParameters);
}
