/*!
	\file CBaseLight.h
	\date 05.05.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../../include/ecs/CBaseComponent.h"
#include "ILight.h"


namespace TDEngine2
{
	/*!
		class CBaseLight

		\brief The class represents a common implementation for all light types that are available within the engine
	*/

	class CBaseLight : public virtual ILight, public CBaseComponent
	{
		public:
			TDE2_REGISTER_TYPE(CBaseLight)

			/*!
				\brief The method assigns color of the light

				\param[in] color A color of the light source

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetColor(const TColor32F& color) override;

			/*!
				\brief The method specifies intensity of the light

				\param[in] intensity An intensity of the light source

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetIntensity(F32 intensity) override;

			/*!
				\brief The method returns a color of the light source

				\return The method returns a color of the light source
			*/

			TDE2_API const TColor32F& GetColor() const override;

			/*!
				\brief The method returns an intensity of the light source

				\return The method returns an intensity of the light source
			*/

			TDE2_API F32 GetIntensity() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseLight)
		protected:
			TColor32F mColor = TColorUtils::mWhite;
			F32       mIntensity = 1.0f;
	};
}