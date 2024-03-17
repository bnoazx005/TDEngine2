/*!
	\file TDEngine2Lighting.inc
	\author Kasimov Ildar
	\date 24.02.2021
*/

#ifndef TDENGINE2_LIGHTING_INC
#define TDENGINE2_LIGHTING_INC


#define MAX_POINT_LIGHTS_COUNT 8
#define MAX_SHADOW_CASCADES_COUNT 4


struct PointLightData
{
	float4   mPosition;
	float4   mColor;
	float    mRange;
	float    mIntensity;
	float2   mUnused;
	float4x4 mLightMats[6];
};


PointLightData CreatePointLight(float4 pos, float4 color, float range, float intensity)
{
	PointLightData newPointLight;

	newPointLight.mPosition = pos;
	newPointLight.mColor = color;
	newPointLight.mRange = range;
	newPointLight.mIntensity = intensity;

	return newPointLight;
}


float PointLightGetAttenuation(in PointLightData pointLight, in float4 pos)
{
	float distance = length(pointLight.mPosition - pos);

	return 1.0 / (distance * distance + 0.00001); //* min(1, max(distance - pointLight.mMaxRange, 0.0));
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
};


LightingData CreateLightingData(float4 pos, float4 normal, float4 viewDir, float4 albedo, float4 materialProperties)
{
	LightingData lightData;
	
	lightData.mPosition = pos;
	lightData.mNormal = normal;
	lightData.mViewDir = viewDir;
	lightData.mAlbedo = albedo;
	lightData.mMaterialProperties = materialProperties;

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


float4 CalcPointLightContribution(in PointLightData pointLight, in LightingData lightingData)
{
	float3 lightDir = normalize(pointLight.mPosition - lightingData.mPosition).xyz;

	float3 radiance = pointLight.mColor.rgb * PointLightGetAttenuation(pointLight, lightingData.mPosition);

	return float4(CalcCookTorranceBRDF(lightDir, lightingData) * radiance, pointLight.mColor.a);
}


float4 CalcSunLightContribution(in SunLightData sunLight, in LightingData lightingData)
{
	return float4(CalcCookTorranceBRDF(normalize(sunLight.mDirection).xyz, lightingData) /* * sunLight.mColor.rgb*/, sunLight.mColor.a);
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