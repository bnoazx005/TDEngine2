#include "./../../include/scene/components/CBaseLight.h"


namespace TDEngine2
{
	CBaseLight::CBaseLight() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CBaseLight::SetColor(const TColor32F& color)
	{
		mColor = color;
		return RC_OK;
	}

	E_RESULT_CODE CBaseLight::SetIntensity(F32 intensity)
	{
		if (intensity < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		mIntensity = intensity;

		return RC_OK;
	}

	const TColor32F& CBaseLight::GetColor() const
	{
		return mColor;
	}

	F32 CBaseLight::GetIntensity() const
	{
		return mIntensity;
	}
}