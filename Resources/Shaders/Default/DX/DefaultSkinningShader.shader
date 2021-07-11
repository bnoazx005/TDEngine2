#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>


struct VertexOut
{
	float4 mPos          : SV_POSITION;
	float4 mLightPos     : POSITION1;
	float4 mWorldPos     : POSITION2;
	float4 mColor        : COLOR;
	float2 mUV           : TEXCOORD;
	float4 mNormal       : NORMAL;
	float3x3 mTBN        : TEXCOORD1;
};


#program vertex

CBUFFER_SECTION(SkinningData)
	float4x4 mJoints[MAX_JOINTS_COUNT];
	uint mUsedJointsCount;
CBUFFER_ENDSECTION


struct VertexIn
{
	float4 mPos     : POSITION0;
	float4 mColor   : COLOR0;
	float2 mUV      : TEXCOORD;
	float4 mNormal  : NORMAL;
	float4 mTangent : TANGENT;
	float4 mJointWeights : BLENDWEIGHT; 
	uint4  mJointIndices : BLENDINDICES;
};


VertexOut mainVS(in VertexIn input)
{
	VertexOut output;

	output.mPos      = mul(mul(ProjMat, mul(ViewMat, ModelMat)), input.mPos);
	output.mLightPos = mul(mul(SunLightMat, ModelMat), input.mPos);
	output.mWorldPos = mul(ModelMat, input.mPos);
	output.mNormal   = mul(transpose(InvModelMat), input.mNormal);
	output.mUV       = input.mUV;
	output.mColor    = input.mColor;

	float3 tangent  = mul(transpose(InvModelMat), input.mTangent);
	float3 binormal = cross(output.mNormal, tangent);

	output.mTBN = transpose(float3x3(tangent, binormal, output.mNormal.xyz));

	return output;
}

#endprogram

#program pixel

#include <TDEngine2ShadowMappingUtils.inc>


DECLARE_TEX2D_EX(AlbedoMap, 1);
DECLARE_TEX2D_EX(NormalMap, 2);
DECLARE_TEX2D_EX(PropertiesMap, 3);


float4 mainPS(VertexOut input): SV_TARGET0
{
	float3 normal = mul(input.mTBN, 2.0 * TEX2D(NormalMap, input.mUV).xyz - 1.0);

	LightingData lightingData = CreateLightingData(input.mWorldPos, float4(normal, 0.0), 
												   normalize(CameraPosition - input.mWorldPos), 
												   GammaToLinear(TEX2D(AlbedoMap, input.mUV)),
												   TEX2D(PropertiesMap, input.mUV));

	float4 sunLight = CalcSunLightContribution(CreateSunLight(SunLightPosition, SunLightDirection, float4(1.0, 1.0, 1.0, 1.0)), lightingData);

	float4 pointLightsContribution = float4(0.0, 0.0, 0.0, 0.0);

	for (int i = 0; i < ActivePointLightsCount; ++i)
	{
		pointLightsContribution += CalcPointLightContribution(PointLights[i], lightingData);
	}

	return (sunLight + pointLightsContribution)	* (1.0 - ComputeShadowFactorPCF(8, input.mLightPos, 0.0001, 1000.0)) * input.mColor;
}

#endprogram
