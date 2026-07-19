/*!
	\file ILight.h
	\date 04.05.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../../utils/Types.h"
#include "../../utils/Config.h"
#include "../../utils/Color.h"
#include "../../math/TVector3.h"
#include "../../ecs/IComponentFactory.h"


namespace TDEngine2
{
	/*!
		struct TBaseLightParameters

		\brief The structure contains parameters for creation of a base light's component
	*/

	typedef struct TBaseLightParameters : public TBaseComponentParameters
	{
		TColor32F mColor = TColorUtils::mWhite;
		F32       mIntensity = 1.0f;
	} TBaseLightParameters;


	/*!
		struct TDirectionalLightParameters

		\brief The structure contains parameters for creation of a directional light's component
	*/

	typedef struct TDirectionalLightParameters : public TBaseLightParameters
	{
		TVector3 mDirection;
	} TDirectionalLightParameters;


	/*!
		struct TPointLightParameters

		\brief The structure contains parameters for creation of a point light's component
	*/

	typedef struct TPointLightParameters : public TBaseLightParameters
	{
		F32 mRange = 1.0f;
	} TPointLightParameters;


	/*!
		struct TSpotLightParameters

		\brief The structure contains parameters for creation of a Spot light's component
	*/

	typedef struct TSpotLightParameters : public TBaseLightParameters
	{
		F32 mAngle = 0.0f;
		F32 mRange = 1.0f;
	} TSpotLightParameters;
}