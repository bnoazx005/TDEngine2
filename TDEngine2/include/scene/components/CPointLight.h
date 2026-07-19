/*!
	\file CPointLight.h
	\date 24.02.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "ILight.h"
#include "../../ecs/CBaseComponent.h"


namespace TDEngine2
{
	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TPointLightComponentData
	{
		FIELD_META(name = color) TColor32F mColor = TColorUtils::mWhite;
		FIELD_META(name = intensity) F32   mIntensity = 1.0f;
		FIELD_META(name = range) F32       mRange = 1.0f;
		TDE2_DECLARE_COMPONENT_META(TPointLightComponentData);
	};


	/*!
		\brief A factory function for creation objects of CPointLight's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CPointLight's implementation
	*/

	TDE2_API IComponent* CreatePointLight(E_RESULT_CODE& result);


	/*!
		class CPointLight

		\brief The class represents omni directional light source 
	*/

	class CPointLight : public CBaseComponentT<CPointLight, TPointLightComponentData>
	{
		public:
			friend TDE2_API IComponent* CreatePointLight(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CPointLight)

			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API const std::string& GetTypeName() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPointLight)
	};


	TDE2_DECLARE_COMPONENT_FACTORY(PointLight, TPointLightParameters);
}
