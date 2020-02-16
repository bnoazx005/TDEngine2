/*!
	\file TDEngine2EffectsUtils.inc
	\author Kasimov Ildar
	\date 15.02.20202

	\brief The file contains definitions of functions, types and etc,
	which are used for post-processing effects
*/

#ifndef TDENGINE2_EFFECTSUTILS_INC
#define TDENGINE2_EFFECTSUTILS_INC


inline float3 GammaToLinear(float3 color, float gammaValue = 2.2) { return pow(color, gammaValue); }
inline float4 GammaToLinear(float4 color, float gammaValue = 2.2) { return float4(pow(color.rgb, gammaValue), color.a); }

inline float3 LinearToGamma(float3 color, float gammaValue = 2.2) { return pow(color, 1.0f / gammaValue); }
inline float4 LinearToGamma(float4 color, float gammaValue = 2.2) { return float4(pow(color.rgb, 1.0f / gammaValue), color.a); }


#endif