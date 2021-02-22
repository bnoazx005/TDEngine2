/*!
	\file TDEngine2ShadowMappingUtils.inc
	\author Kasimov Ildar
	\date 22.02.2021

	\brief The file contains helper functions to simplify code related with shadow mapping techniques
*/

#ifndef TDENGINE2_SHADOW_MAPPING_UTILS_INC
#define TDENGINE2_SHADOW_MAPPING_UTILS_INC


#ifdef TDE2_HLSL_SHADER

static const uint PoissonDiskSamplesCount = 16;

static float2 PoissonDisk[PoissonDiskSamplesCount] =
{
	float2(0.2770745, 0.6951455),
	float2(0.1874257, -0.02561589),
	float2(-0.3381929, 0.8713168),
	float2(0.5867746, 0.1087471),
	float2(-0.3078699, 0.188545),
	float2(0.7993396, 0.4595091),
	float2(-0.09242552, 0.5260149),
	float2(0.3657553, -0.5329605),
	float2(-0.3829718, -0.2476171),
	float2(-0.01085108, -0.6966301),
	float2(0.8404155, -0.3543923),
	float2(-0.5186161, -0.7624033),
	float2(-0.8135794, 0.2328489),
	float2(-0.784665, -0.2434929),
	float2(0.9920505, 0.0855163),
	float2(-0.687256, 0.6711345)
};


uint getRandomIndex(float4 seed, uint minValue, uint maxValue)
{
    return (uint)floor((maxValue - minValue) * frac(sin(dot(seed, float4(12.9898,78.233,45.164,94.673))) * 43758.5453)) + minValue;
}

#endif


#ifdef TDE2_GLSL_SHADER

const int PoissonDiskSamplesCount = 16;

const vec2 PoissonDisk[PoissonDiskSamplesCount] = vec2[PoissonDiskSamplesCount]
(
	vec2(0.2770745, 0.6951455),
	vec2(0.1874257, -0.02561589),
	vec2(-0.3381929, 0.8713168),
	vec2(0.5867746, 0.1087471),
	vec2(-0.3078699, 0.188545),
	vec2(0.7993396, 0.4595091),
	vec2(-0.09242552, 0.5260149),
	vec2(0.3657553, -0.5329605),
	vec2(-0.3829718, -0.2476171),
	vec2(-0.01085108, -0.6966301),
	vec2(0.8404155, -0.3543923),
	vec2(-0.5186161, -0.7624033),
	vec2(-0.8135794, 0.2328489),
	vec2(-0.784665, -0.2434929),
	vec2(0.9920505, 0.0855163),
	vec2(-0.687256, 0.6711345)
);


int getRandomIndex(vec4 seed, int minValue, int maxValue)
{
    return int(floor((maxValue - minValue) * fract(sin(dot(seed, vec4(12.9898,78.233,45.164,94.673))) * 43758.5453))) + minValue;
}

#endif


DECLARE_TEX2D(DirectionalShadowMapTexture);


/*!
	\brief HLSL version of functions
*/

#ifdef TDE2_HLSL_SHADER

float ComputeShadowFactor(float4 lightSpaceFragPos, float bias)
{
	float3 projectedPos = lightSpaceFragPos.xyz / lightSpaceFragPos.w;

	if (projectedPos.z > 1.0)
	{
		return 0.0;
	}

	projectedPos.x = 0.5 * projectedPos.x + 0.5;
	projectedPos.y = -0.5 * projectedPos.y + 0.5;

	return ((projectedPos.z - bias) > TEX2D(DirectionalShadowMapTexture, projectedPos.xy) ? 0.9 : 0.0);
}


float ComputeShadowFactorPCF(uint samplesCount, float4 lightSpaceFragPos, float bias, float spread)
{
	samplesCount = clamp(samplesCount, 0, PoissonDiskSamplesCount); //clamp samplesCount to [0; maxSamplesCount] range

	float3 projectedPos = lightSpaceFragPos.xyz / lightSpaceFragPos.w;

	if (projectedPos.z > 1.0)
	{
		return 0.0;
	}

	projectedPos.x = 0.5 * projectedPos.x + 0.5;
	projectedPos.y = -0.5 * projectedPos.y + 0.5;

	float shadowFactor = 0.0;

	uint randomIndex = 0;

	[unroll]
	for (int i = 0; i < samplesCount; ++i)
	{
		randomIndex = getRandomIndex(float4(lightSpaceFragPos.xyz, i), 0, PoissonDiskSamplesCount);

		shadowFactor += ((projectedPos.z - bias) > TEX2D(DirectionalShadowMapTexture, projectedPos.xy + PoissonDisk[randomIndex] / spread) ? 0.9 : 0.0);
	}

	shadowFactor /= samplesCount;

	return shadowFactor;
}

#endif


/*!
	\brief GLSL version of functions
*/

#ifdef TDE2_GLSL_SHADER

float ComputeShadowFactor(vec4 lightSpaceFragPos, float bias)
{
	vec3 projectedPos = lightSpaceFragPos.xyz / lightSpaceFragPos.w;

	if (projectedPos.z > 1.0)
	{
		return 0.0;
	}

	projectedPos = projectedPos * 0.5 + 0.5;

	return ((projectedPos.z - bias) > TEX2D(DirectionalShadowMapTexture, projectedPos.xy).r ? 0.9 : 0.0);
}


float ComputeShadowFactorPCF(int samplesCount, vec4 lightSpaceFragPos, float bias, float spread)
{
	samplesCount = clamp(samplesCount, 0, PoissonDiskSamplesCount); //clamp samplesCount to [0; maxSamplesCount] range

	vec3 projectedPos = lightSpaceFragPos.xyz / lightSpaceFragPos.w;

	if (projectedPos.z > 1.0)
	{
		return 0.0;
	}

	projectedPos = projectedPos * 0.5 + 0.5;

	float shadowFactor = 0.0;

	int randomIndex = 0;

	for (int i = 0; i < samplesCount; ++i)
	{
		randomIndex = getRandomIndex(vec4(lightSpaceFragPos.xyz, i), 0, PoissonDiskSamplesCount);

		shadowFactor += ((projectedPos.z - bias) > TEX2D(DirectionalShadowMapTexture, projectedPos.xy + PoissonDisk[randomIndex] / spread).r ? 0.9 : 0.0);
	}

	shadowFactor /= samplesCount;

	return shadowFactor;
}

#endif

#endif