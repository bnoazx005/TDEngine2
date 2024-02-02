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
	return pow(color, float3(1.0 / gammaValue)); 
#else 
	return pow(color, 1.0 / gammaValue); 
#endif
}

float4 LinearToGamma(float4 color, float gammaValue = 2.2) 
{
	return float4(LinearToGamma(color.rgb, 1.0 / gammaValue), color.a); 
}


/*!
	\brief Color grading functions 
*/


// Function works with the global state. Use ColorGradingLUT texture and its sampler ColorGradingLUT_SamplerState
/*float4 ApplyGrading(in float4 color, in Texture2D lut, in SamplerState samplerState)
{
	float2 uv = float2(0.2, 0.0);
	return color ;//* lut.Sample(samplerState, uv);
}
*/


float3 CalcLuminance(float3 color) { return max(1e-4, dot(float3(0.299, 0.587, 0.114), color)); }


#endif