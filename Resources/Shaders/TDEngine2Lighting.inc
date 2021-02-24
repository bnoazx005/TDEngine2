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


struct LightingData
{
	float4 mPosition;
	float4 mNormal;
	float4 mViewDir;
	float4 mAlbedo;
	float4 mMaterialProperties; // x - roughness, y - metalness
};


float PointLightGetAttenuation(in PointLightData pointLight, in float4 pos)
{
	float distance = length(pointLight.mPosition - pos);

	return 1.0 / (distance * distance); //* min(1, max(distance - pointLight.mMaxRange, 0.0));
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
	return float4(0.0, 0.0, 0.0, 0.0);
}


#endif