/*!
	\file TDEngine2Lighting.inc
	\author Kasimov Ildar
	\date 24.02.2021
*/

#ifndef TDENGINE2_LIGHTING_INC
#define TDENGINE2_LIGHTING_INC


#define MAX_POINT_LIGHTS_COUNT 8


struct PointLightData
{
	float4 mPosition;
	float4 mColor;
	float  mRange;
	float  mIntensity;
	float2 mUnused;	
};


PointLightData CreatePointLight(float4 pos, float4 color, float range, float intensity)
{
	PointLightData output;

	output.mPosition = pos;
	output.mColor = color;
	output.mRange = range;
	output.mIntensity = intensity;

	return output;
}


float PointLightGetAttenuation(in PointLightData pointLight, in float4 pos)
{
	float distance = length(pointLight.mPosition - pos);

	return 1.0 / (distance * distance); //* min(1, max(distance - pointLight.mMaxRange, 0.0));
}


struct SunLightData
{	
	float4 mDirection;
	float4 mPosition;
	float4 mColor;
};


SunLightData CreateSunLight(float4 pos, float4 dir, float4 color)
{
	SunLightData output;
	
	output.mDirection = dir;
	output.mPosition = pos;
	output.mColor = color;

	return output;
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
	LightingData output;
	
	output.mPosition = pos;
	output.mNormal = normal;
	output.mViewDir = viewDir;
	output.mAlbedo = albedo;
	output.mMaterialProperties = materialProperties;

	return output;
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

	return NdotV / NdotV * (1.0 - k) + k;
}


float ComputeGeometrySmith(float3 normal, float3 viewDir, float3 lightDir, float k)
{
	float normalViewDirDotProduct  = max(dot(normal, viewDir), 0.0);
	float normalLightDirDotProduct = max(dot(normal, lightDir), 0.0);

	return ComputeGeometrySchlick(normalViewDirDotProduct, k) *
		   ComputeGeometrySchlick(normalLightDirDotProduct, k);
}


float3 ComputeFresnelSchlickCoeff(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}


float4 CalcPointLightContribution(in PointLightData pointLight, in LightingData lightingData)
{
	float3 lightDir = normalize(pointLight.mPosition.xyz - lightingData.mPosition.xyz);

	float3 normal = lightingData.mNormal,xyz;

	float normalLightDirDotProduct = max(dot(normal, lightDir), 0.0);

	return normalLightDirDotProduct * pointLight.mColor * PointLightGetAttenuation(pointLight, lightingData.mPosition);
}


float4 CalcSunLightContribution(in SunLightData sunLight, in LightingData lightingData)
{
	float3 lightDir = normalize(sunLight.mPosition);

	float3 normal = lightingData.mNormal,xyz;
	float3 view   = lightingData.mViewDir.xyz;
	float3 albedo = lightingData.mAlbedo.rgb;

	float3 halfVec = normalize(view + lightDir);

	const float roughness = lightingData.mMaterialProperties.x;
	const float metalness = lightingData.mMaterialProperties.y;

	float NDF = ComputeDistribution(normal, halfVec, roughness);
	float G   = ComputeGeometrySmith(normal, view, lightDir, roughness);

	float3 F0 =  lerp(0.04, albedo, metalness); // equals to 0.04 for all dielectrics

	float3 F = ComputeFresnelSchlickCoeff(max(dot(view, halfVec), 0.0), F0);
	
	float normalLightDirDotProduct = max(dot(normal, lightDir), 0.0);

	float3 specular = NDF * G * F / (4.0 * max(dot(normal, view), 0.0) * normalLightDirDotProduct + 0.001);

	float3 kS = F;
	float3 kD = 1.0 - kS;	

	kD *= (1.0 - metalness);

	return float4(normalLightDirDotProduct * albedo.rgb, 1.0);
	//return float4((kD * albedo / Pi + specular) * normalLightDirDotProduct * sunLight.mColor.rgb, sunLight.mColor.a);
}


#endif