#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS


#include <TDEngine2Globals.inc>


struct VertexOut
{
	float4 mPos              : SV_POSITION;
	float4 mLightPos         : POSITION1;
	float4 mWorldPos         : POSITION2;
	float4 mColor            : COLOR;
	float2 mUV               : TEXCOORD;
	float4 mNormal           : NORMAL;
	float3x3 mTangentToWorld : TEXCOORD1;
	float4 mTangentViewDir   : TEXCOORD4;
};


#program vertex

struct VertexIn
{
	float4 mPos     : POSITION0;
	float4 mColor   : COLOR0;
	float2 mUV      : TEXCOORD;
	float4 mNormal  : NORMAL;
	float4 mTangent : TANGENT;
};


VertexOut mainVS(in VertexIn input)
{
	VertexOut output;

	output.mPos      = mul(mul(ProjMat, mul(ViewMat, ModelMat)), input.mPos);
	output.mLightPos = mul(mul(SunLightMat, ModelMat), input.mPos);
	output.mWorldPos = mul(ModelMat, input.mPos);
	output.mNormal   = normalize(mul(ModelMat, input.mNormal));
	output.mUV       = input.mUV;
	output.mColor    = input.mColor;

	float3 tangent  = normalize(mul(ModelMat, input.mTangent));
	float3 binormal = normalize(cross(output.mNormal, tangent));

	output.mTangentToWorld = transpose(float3x3(tangent, binormal, output.mNormal.xyz));

	float3 view = CameraPosition.xyz - output.mWorldPos.xyz;
	output.mTangentViewDir = float4(dot(tangent, view), dot(view, binormal), dot(output.mNormal.xyz, view), 0.0);

	return output;
}

#endprogram

#program pixel

#include <TDEngine2ShadowMappingUtils.inc>


DECLARE_TEX2D_EX(AlbedoMap, 1);
DECLARE_TEX2D_EX(NormalMap, 2);
DECLARE_TEX2D_EX(PropertiesMap, 3);


TDE2_ENABLE_PARALLAX_MAPPING


CBUFFER_SECTION_EX(ShaderParameters, 4)
	float parallaxMappingEnabled;
CBUFFER_ENDSECTION


float4 mainPS(VertexOut input): SV_TARGET0
{
	float2 uv = lerp(input.mUV, CalcParallaxMappingOffset(input.mUV, normalize(input.mTangentViewDir).xyz, normalize(input.mNormal), 0.2, 8.0, 32.0), parallaxMappingEnabled);
	TDE2_DISCARD_PIXELS(uv);

	float3 normal = mul(input.mTangentToWorld, 2.0 * TEX2D(NormalMap, uv).xyz - 1.0);

	LightingData lightingData = CreateLightingData(input.mWorldPos, float4(normal, 0.0), 
												   normalize(CameraPosition - input.mWorldPos), 
												   GammaToLinear(TEX2D(AlbedoMap, uv)),
												   TEX2D(PropertiesMap, uv));

	float4 sunLight = CalcSunLightContribution(CreateSunLight(SunLightPosition, SunLightDirection, float4(1.0, 1.0, 1.0, 1.0)), lightingData);

	float4 pointLightsContribution = float4(0.0, 0.0, 0.0, 0.0);

	for (int i = 0; i < ActivePointLightsCount; ++i)
	{
		pointLightsContribution += CalcPointLightContribution(PointLights[i], lightingData);
	}

	return (sunLight + pointLightsContribution)	* (1.0 - ComputeShadowFactorPCF(8, input.mLightPos, 0.0001, 1000.0)) * input.mColor;
}

#endprogram
