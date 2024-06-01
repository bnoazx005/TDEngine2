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

static float4 ShadowCascadesDebugOutput[MAX_SHADOW_CASCADES_COUNT] =
{
	float4(1.0, 0.0, 0.0, 1.0),
	float4(0.0, 1.0, 0.0, 1.0),
	float4(0.0, 0.0, 1.0, 1.0),
	float4(1.0, 1.0, 1.0, 1.0)	
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


const vec4 ShadowCascadesDebugOutput[MAX_SHADOW_CASCADES_COUNT] = vec4[MAX_SHADOW_CASCADES_COUNT]
(
	float4(1.0, 0.0, 0.0, 1.0),
	float4(0.0, 1.0, 0.0, 1.0),
	float4(0.0, 0.0, 1.0, 1.0),
	float4(1.0, 1.0, 1.0, 1.0)	
);


int getRandomIndex(vec4 seed, int minValue, int maxValue)
{
    return int(floor((maxValue - minValue) * fract(sin(dot(seed, vec4(12.9898,78.233,45.164,94.673))) * 43758.5453))) + minValue;
}

#endif


DECLARE_TEX2DARRAY_EX(DirectionalShadowMapTexture, 12);
DECLARE_CUBETEX_EX(PointLightShadowMapTexture_0, 13);
DECLARE_CUBETEX_EX(PointLightShadowMapTexture_1, 14);


int GetSunShadowCascadeIndex(float4 fragViewPos)
{
#ifdef TDE2_GLSL_SHADER
	float cascadeSplits[4] = float[]
	(
		ShadowCascadesSplits.x,
		ShadowCascadesSplits.y,
		ShadowCascadesSplits.z,
		ShadowCascadesSplits.w
	);
#endif

#ifdef TDE2_HLSL_SHADER
	const float cascadeSplits[4] =
	{
		ShadowCascadesSplits.x,
		ShadowCascadesSplits.y,
		ShadowCascadesSplits.z,
		ShadowCascadesSplits.w
	};
#endif

	float depth = abs(fragViewPos.z);

	for (int i = 0; i < ShadowCascadesCount; i++)
	{
		if (depth < cascadeSplits[i])
		{
			return i;
		}
	}

	return ShadowCascadesCount - 1;
}


/*!
	\brief HLSL version of functions
*/

#ifdef TDE2_HLSL_SHADER

float ComputeSunShadowFactor(float4 worldSpaceFragPos, float bias, int cascadeIndex)
{
	float4 lightSpaceFragPos = mul(SunLightMat[cascadeIndex], worldSpaceFragPos);
	float3 projectedPos = lightSpaceFragPos.xyz / lightSpaceFragPos.w;

	if (projectedPos.z > 1.0)
	{
		return 0.0;
	}

	projectedPos.x = 0.5 * projectedPos.x + 0.5;
	projectedPos.y = -0.5 * projectedPos.y + 0.5;

	return lerp(1.0, (projectedPos.z - bias) > TEX2D(DirectionalShadowMapTexture, float3(projectedPos.xy, cascadeIndex)) ? 0.9 : 0.0, ShadowMapsEnabled);
}


float ComputeSunShadowFactorPCF(uint samplesCount, int cascadeIndex, float4 worldSpaceFragPos, float bias, float spread)
{
	samplesCount = clamp(samplesCount, 0, PoissonDiskSamplesCount); //clamp samplesCount to [0; maxSamplesCount] range

	float4 lightSpaceFragPos = mul(SunLightMat[cascadeIndex], worldSpaceFragPos);
	float3 projectedPos = lightSpaceFragPos.xyz / lightSpaceFragPos.w;

	if (projectedPos.z > 1.0 || !ShadowMapsEnabled)
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
		float2 uv = projectedPos.xy + PoissonDisk[randomIndex] / spread;

		shadowFactor += ((projectedPos.z - bias) > TEX2D(DirectionalShadowMapTexture, float3(uv.x, uv.y, (float)cascadeIndex)) ? 0.9 : 0.0);
	}

	shadowFactor /= samplesCount;

	return shadowFactor;
}

#endif


/*!
	\brief GLSL version of functions
*/

#ifdef TDE2_GLSL_SHADER

float ComputeSunShadowFactor(vec4 worldSpaceFragPos, float bias, int cascadeIndex)
{
	float4 lightSpaceFragPos = mul(SunLightMat[cascadeIndex], worldSpaceFragPos);
	vec3 projectedPos = lightSpaceFragPos.xyz / lightSpaceFragPos.w;

	if (projectedPos.z > 1.0)
	{
		return 0.0;
	}

	projectedPos = projectedPos * 0.5 + 0.5;

	return ((projectedPos.z - bias) > TEX2D(DirectionalShadowMapTexture, vec3(projectedPos.xy, cascadeIndex)).r ? 0.9 : 0.0);
}


float ComputeSunShadowFactorPCF(int samplesCount, int cascadeIndex, vec4 worldSpaceFragPos, float bias, float spread)
{
	samplesCount = clamp(samplesCount, 0, PoissonDiskSamplesCount); //clamp samplesCount to [0; maxSamplesCount] range

	float4 lightSpaceFragPos = mul(SunLightMat[cascadeIndex], worldSpaceFragPos);
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

		shadowFactor += ((projectedPos.z - bias) > TEX2D(DirectionalShadowMapTexture, vec3(projectedPos.xy + PoissonDisk[randomIndex] / spread, cascadeIndex)).r ? 0.9 : 0.0);
	}

	shadowFactor /= samplesCount;

	return shadowFactor;
}

#endif


float ComputePointLightShadowFactor(in LightSourceData pointLight, float4 fragPos, float bias)
{	
	float3 lightDir = (fragPos - pointLight.mPosition).xyz;
	float closestDepth = pointLight.mRange * TEXCUBE(PointLightShadowMapTexture_0, lightDir).r;
	return (length(lightDir) - bias) >  closestDepth ? 1.0 : 0.0;
}

#endif