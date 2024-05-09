/*!
	\file TDEngine2Lighting.inc
	\author Kasimov Ildar
	\date 24.02.2021
*/

#ifndef TDENGINE2_LIGHTING_INC
#define TDENGINE2_LIGHTING_INC


#define MAX_POINT_LIGHTS_COUNT 8
#define MAX_SPOT_LIGHTS_COUNT 8
#define MAX_SHADOW_CASCADES_COUNT 4


#define POINT_LIGHT_TYPE 0
#define SPOT_LIGHT_TYPE 1


#ifdef TDE2_HLSL_SHADER
#define TILE_INDEX_TYPE uint2
#else
#define TILE_INDEX_TYPE ivec2
#endif


struct LightSourceData
{
	float4   mPosition;
	float4   mDirection;
	float4   mColor;
	float    mIntensity;
	float    mRange;
	float    mAngle;
	float    mUnused;
    int      mType;

#if TDE2_HLSL_SHADER
    int3     mUnused2;
#endif

	float4x4 mLightMats[6];
};


DECLARE_TYPED_BUFFER_EX(LightSourceData, Lights, 10);

#ifdef TILED_LIGHTING_ENABLED

#define LIGHT_TILE_BLOCK_SIZE 16
#define MAX_LIGHTS_PER_TILE 256

DECLARE_TEX2D_TYPED_EX(LightGridTexture, 11, uint2);
DECLARE_TYPED_BUFFER_EX(uint, VisibleLightIndices, 12);

#endif


float GetLightAttenuation(in LightSourceData light, in float4 pos)
{
	float distance = length(light.mPosition - pos);

	//return 1.0 / (5.0 * distance * distance + 0.00001); // \note Correct PBR based approach
	return 1.0 - smoothstep(light.mRange * 0.75f, light.mRange, distance);
}


struct SunLightData
{	
	float4 mDirection;
	float4 mPosition;
	float4 mColor;
};


SunLightData CreateSunLight(float4 pos, float4 dir, float4 color)
{
	SunLightData sunLight;
	
	sunLight.mDirection = dir;
	sunLight.mPosition = pos;
	sunLight.mColor = color;

	return sunLight;
}


struct LightingData
{
	float4 mPosition;
	float4 mNormal;
	float4 mViewDir;
	float4 mAlbedo;
	float4 mMaterialProperties; // x - roughness, y - metalness

	TILE_INDEX_TYPE mTileIndex;
};


LightingData CreateLightingData(float4 pos, float4 normal, float4 viewDir, float4 albedo, float4 materialProperties, float4 fragPos)
{
	LightingData lightData;
	
	lightData.mPosition = pos;
	lightData.mNormal = normal;
	lightData.mViewDir = viewDir;
	lightData.mAlbedo = albedo;
	lightData.mMaterialProperties = materialProperties;

#ifdef TILED_LIGHTING_ENABLED
	lightData.mTileIndex = TILE_INDEX_TYPE(floor(fragPos.xy / LIGHT_TILE_BLOCK_SIZE));
#endif

	return lightData;
}


/*!
	\brief Physically Based Shading functions 
*/

float ComputeDistribution(float3 normal, float3 halfVector, float roughness)
{
	float alpha = roughness * roughness;

	float squaredAlpha = alpha * alpha;

	float normalHalfVectorDotProduct = max(dot(normal, halfVector), 0.0);
	float squaredDotProduct          = normalHalfVectorDotProduct * normalHalfVectorDotProduct;

	float denomExpr = (squaredDotProduct * (squaredAlpha - 1.0) + 1.0);

	denomExpr = Pi * denomExpr * denomExpr;

	return squaredAlpha / denomExpr;
}


float ComputeGeometrySchlick(float NdotV, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0;

	return NdotV / (NdotV * (1.0 - k) + k);
}


float ComputeGeometrySmith(float3 normal, float3 viewDir, float3 lightDir, float k)
{
	float normalViewDirDotProduct  = max(dot(normal, viewDir), 0.0);
	float normalLightDirDotProduct = max(dot(normal, lightDir), 0.0);

	return ComputeGeometrySchlick(normalViewDirDotProduct, k) * ComputeGeometrySchlick(normalLightDirDotProduct, k);
}


float3 ComputeFresnelSchlickCoeff(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}


float3 CalcCookTorranceBRDF(float3 lightDir, in LightingData lightingData)
{
	float3 normal = lightingData.mNormal.xyz;
	float3 view   = lightingData.mViewDir.xyz;
	float3 albedo = lightingData.mAlbedo.rgb;

	float3 halfVec = normalize(view + lightDir);

	float roughness = lightingData.mMaterialProperties.x;
	float metalness = lightingData.mMaterialProperties.y;

	float NDF = ComputeDistribution(normal, halfVec, roughness);
	float G   = ComputeGeometrySmith(normal, view, lightDir, roughness);

#if TDE2_HLSL_SHADER
	float3 F = ComputeFresnelSchlickCoeff(max(dot(view, halfVec), 0.0), lerp(0.04, albedo, metalness)); // equals to 0.04 for all dielectrics
#else
	float3 F = ComputeFresnelSchlickCoeff(max(dot(view, halfVec), 0.0), mix(vec3(0.04), albedo, metalness)); // equals to 0.04 for all dielectrics
#endif

	float normalLightDirDotProduct = max(dot(normal, lightDir), 0.0);

	float3 specular = NDF * G * F / (4.0 * max(dot(normal, view), 0.0) * normalLightDirDotProduct + 0.001);

	float3 kS = F;
	float3 kD = 1.0 - kS;	

	kD *= (1.0 - metalness);

	return (kD * albedo / Pi + specular) * normalLightDirDotProduct;
}


float4 CalcPointLightContribution(in LightSourceData pointLight, in LightingData lightingData)
{
	float3 lightDir = normalize(pointLight.mPosition - lightingData.mPosition).xyz;

	float3 radiance = pointLight.mColor.rgb * GetLightAttenuation(pointLight, lightingData.mPosition);

	return float4(CalcCookTorranceBRDF(lightDir, lightingData) * radiance, pointLight.mColor.a);
}


float4 CalcSpotLightContribution(in LightSourceData spotLight, in LightingData lightingData)
{
	float3 lightDir = normalize(spotLight.mPosition - lightingData.mPosition).xyz;

	float minCos = cos(radians(spotLight.mAngle));

#if TDE2_HLSL_SHADER
	float maxCos = lerp(minCos, 1.0, 0.5);
#else
	float maxCos = mix(minCos, 1.0, 0.5);
#endif	

	float intensity = smoothstep(minCos, maxCos, max(0.0, dot(spotLight.mDirection.xyz, -lightDir)));
	float3 radiance = spotLight.mColor.rgb * intensity * GetLightAttenuation(spotLight, lightingData.mPosition);

	return float4(CalcCookTorranceBRDF(lightDir, lightingData) * radiance, spotLight.mColor.a);
}


float4 CalcSunLightContribution(in SunLightData sunLight, in LightingData lightingData)
{
	return float4(CalcCookTorranceBRDF(normalize(sunLight.mDirection).xyz, lightingData) /* * sunLight.mColor.rgb*/, sunLight.mColor.a);
}


float ComputePointLightShadowFactor(in LightSourceData pointLight, float4 fragPos, float bias);


float4 CalcLightsContribution(in LightingData lightingData, int activeLightsCount)
{
	float4 lightContribution = float4(0.0, 0.0, 0.0, 0.0);

#ifdef TILED_LIGHTING_ENABLED
	
	uint startOffset = FETCH_TEX2D_LOD(LightGridTexture, lightingData.mTileIndex, 0).x;
	uint lightsCount = FETCH_TEX2D_LOD(LightGridTexture, lightingData.mTileIndex, 0).y;

#ifdef TDE2_HLSL_SHADER
	[unroll(16)]
#endif
	for (int i = 0; i < int(lightsCount); ++i)
	{
		uint lightIndex = TYPED_BUFFER_ACCESS(VisibleLightIndices)[startOffset + uint(i)];
		LightSourceData currLight = TYPED_BUFFER_ACCESS(Lights)[lightIndex];

		if (length(currLight.mPosition - lightingData.mPosition) < currLight.mRange)
		{
			switch (currLight.mType)
			{
				case POINT_LIGHT_TYPE:
					lightContribution += CalcPointLightContribution(currLight, lightingData) * (1.0 - ComputePointLightShadowFactor(currLight, lightingData.mPosition, 0.1));
					break;
				case SPOT_LIGHT_TYPE:
					lightContribution += CalcSpotLightContribution(currLight, lightingData);
					break;
			}
		}
	}

#else

	for (int i = 0; i < activeLightsCount; ++i)
	{
		if (length(TYPED_BUFFER_ACCESS(Lights)[i].mPosition - lightingData.mPosition) < TYPED_BUFFER_ACCESS(Lights)[i].mRange)
		{
			switch (TYPED_BUFFER_ACCESS(Lights)[i].mType)
			{
				case POINT_LIGHT_TYPE:
					lightContribution += CalcPointLightContribution(TYPED_BUFFER_ACCESS(Lights)[i], lightingData) * (1.0 - ComputePointLightShadowFactor(TYPED_BUFFER_ACCESS(Lights)[i], lightingData.mPosition, 0.1));
					break;
				case SPOT_LIGHT_TYPE:
					lightContribution += CalcSpotLightContribution(TYPED_BUFFER_ACCESS(Lights)[i], lightingData);
					break;
			}
		}
	}
#endif

	return lightContribution;
}

	
	#ifndef TDE2_ENABLE_PARALLAX_MAPPING

		#if TDE2_HLSL_SHADER
			#define TDE2_LERP lerp
		#else
			#define TDE2_LERP mix
		#endif

		#define TDE2_ENABLE_PARALLAX_MAPPING 																												\
			float2 CalcParallaxMappingOffset(float2 uv, float3 viewDir, float3 normal, float heightScale, float minLayers, float maxLayers)					\
			{																																				\
				float numLayers = TDE2_LERP(maxLayers, minLayers, abs(dot(normal, viewDir)));																\
																																							\
				float layerDepth = 1.0 / numLayers;																											\
				float currLayerDepth = 0.0;																													\
																																							\
				float2 deltaUVs = ((viewDir.xy / viewDir.z) * heightScale) / numLayers;																		\
				float2 outputUV = uv;																														\
																																							\
				float currDepth = TEX2D(PropertiesMap, uv).b;																								\
																																							\
				for (int i = 0; i < numLayers; ++i)																											\
				{																																			\
					if (currLayerDepth > currDepth)																											\
					{																																		\
						break;																																\
					}																																		\
																																							\
					outputUV -= deltaUVs;																													\
					currDepth = TEX2D(PropertiesMap, outputUV).b;																							\
					currLayerDepth += layerDepth;																											\
				}																																			\
																																							\
				float2 prevUVs = outputUV + deltaUVs;																										\
																																							\
				float afterDepth  = currDepth - currLayerDepth;																								\
				float beforeDepth = TEX2D(PropertiesMap, prevUVs).b - currLayerDepth + layerDepth;															\
			 																																				\
				float weight = afterDepth / (afterDepth - beforeDepth);																						\
				return prevUVs * weight + outputUV * (1.0 - weight);																						\
			}


	#endif

	#ifndef TDE2_DISCARD_PIXELS
		#define TDE2_DISCARD_PIXELS(uv) if (uv.x > 1.0 || uv.y > 1.0 || uv.x < 0.0 || uv.y < 0.0) discard
	#endif

#endif