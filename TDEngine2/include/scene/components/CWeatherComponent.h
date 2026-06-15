/*!
	\file CWeatherComponent.h
	\date 06.02.2024
	\authors Kasimov Ildar
*/

#pragma once


#include "../../utils/Types.h"
#include "../../utils/Color.h"
#include "../../core/memory/CPoolAllocator.h"
#include "../../ecs/CBaseComponent.h"


namespace TDEngine2
{
	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TWeatherComponentData
	{
		FIELD_META(name = ambient_clouds_color) TColor32F mAmbientCloudColor = TColor32F(0.41f, 0.41f, 0.41f, 1.0f);

		FIELD_META(name = absorption) F32                 mSunLightAbsorption = 0.002f;

		// clouds
		FIELD_META(name = atmo_start_radius) F32          mAtmosphereStartRadius = 1500.0f;
		FIELD_META(name = atmo_thickness) F32             mAtmosphereThickness = 14000.0f;
		FIELD_META(name = wind_scale) F32                 mWindScaleFactor = 2500.0f;

		FIELD_META(name = coverage) F32                   mCoverage = 0.3f;
		FIELD_META(name = curliness) F32                  mCurliness = 1.1f;
		FIELD_META(name = crispiness) F32                 mCrispiness = 43.0f;

		FIELD_META(name = density_factor) F32             mDensityFactor = 0.006f;

		FIELD_META(name = wind_direction) TVector2        mWindDirection = RightVector2;

		FIELD_META(name = weather_texture) std::string    mWeatherMapTextureId;

		TDE2_DECLARE_COMPONENT_META(TWeatherComponentData);
	};


	/*!
		\brief A factory function for creation objects of CWeatherComponent's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IComponent's implementation
	*/

	TDE2_API IComponent* CreateWeatherComponent(E_RESULT_CODE& result);


	class CWeatherComponent : public CBaseComponentT<CWeatherComponent, TWeatherComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateWeatherComponent(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CWeatherComponent)

			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API const std::string& GetTypeName() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CWeatherComponent)
	};


	typedef struct TWeatherComponentParameters : public TBaseComponentParameters
	{
	} TWeatherComponentParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(WeatherComponent, TWeatherComponentParameters);
}