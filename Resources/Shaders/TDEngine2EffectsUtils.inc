/*!
	\file TDEngine2EffectsUtils.inc
	\author Kasimov Ildar
	\date 15.02.20202

	\brief The file contains definitions of functions, types and etc,
	which are used for post-processing effects
*/

#ifndef TDENGINE2_EFFECTSUTILS_INC
#define TDENGINE2_EFFECTSUTILS_INC


float3 GammaToLinear(float3 color, float gammaValue = 2.2) 
{ 
#ifdef TDE2_GLSL_SHADER
	return pow(color, float3(gammaValue)); 
#else	
	return pow(color, gammaValue); 
#endif
}

float4 GammaToLinear(float4 color, float gammaValue = 2.2) 
{ 
	return float4(GammaToLinear(color.rgb, gammaValue), color.a); 
}

float3 LinearToGamma(float3 color, float gammaValue = 2.2) 
{
#ifdef TDE2_GLSL_SHADER
	return pow(color, float3(1.0f / gammaValue)); 
#else 
	return pow(color, 1.0f / gammaValue); 
#endif
}

float4 LinearToGamma(float4 color, float gammaValue = 2.2) 
{
	return float4(LinearToGamma(color.rgb, 1.0f / gammaValue), color.a); 
}


#endif