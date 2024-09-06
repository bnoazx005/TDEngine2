#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>


struct VertexOut
{
	float4 mPos          : SV_POSITION;
	float4 mLightPos     : POSITION1;
	float4 mWorldPos     : POSITION2;
	float4 mViewWorldPos : POSITION3;
	float4 mColor        : COLOR;
	float2 mUV           : TEXCOORD;
	float4 mNormal       : NORMAL;
	float3x3 mTBN        : TEXCOORD1;
};


#program vertex

#include <TDEngine2SkinningUtils.inc>


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

	float weights[MAX_VERTS_PER_JOINT];
	weights[0] = input.mJointWeights.x;
	weights[1] = input.mJointWeights.y;
	weights[2] = input.mJointWeights.z;
	weights[3] = input.mJointWeights.w;

	float4x4 jointMats[MAX_VERTS_PER_JOINT];
	jointMats[0] = mJoints[input.mJointIndices.x];
	jointMats[1] = mJoints[input.mJointIndices.y];
	jointMats[2] = mJoints[input.mJointIndices.z];
	jointMats[3] = mJoints[input.mJointIndices.w];

	float3 localPos     = float3(0.0, 0.0, 0.0);
	float3 localNormal  = float3(0.0, 0.0, 0.0);
	float3 localTangent = float3(0.0, 0.0, 0.0);

	for (int i = 0; i < MAX_VERTS_PER_JOINT; ++i)
	{
		localPos     += (mul(jointMats[i], input.mPos) * weights[i]).xyz;
		localNormal  += (mul(jointMats[i], input.mNormal) * weights[i]).xyz;
		localTangent += (mul(jointMats[i], input.mTangent) * weights[i]).xyz;
	}

	output.mPos      = mul(mul(ProjMat, mul(ViewMat, ModelMat)), float4(localPos, 1.0));
	output.mLightPos = mul(mul(SunLightMat[0], ModelMat), input.mPos);
	output.mWorldPos = mul(ModelMat, float4(localPos, 1.0));
	output.mViewWorldPos = mul(ViewMat, output.mWorldPos);
	output.mNormal   = mul(transpose(InvModelMat), float4(localNormal, 0.0));
	output.mUV       = input.mUV;
	output.mColor    = input.mColor;

	float3 tangent  = mul(transpose(InvModelMat), float4(localNormal, 0.0));
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
												   TEX2D(PropertiesMap, input.mUV), 
												   input.mPos);

	float4 sunLight = CalcSunLightContribution(CreateSunLight(SunLightPosition, SunLightDirection, float4(1.0, 1.0, 1.0, 1.0)), lightingData);

	return (sunLight * (1.0 - ComputeSunShadowFactorPCF(8, GetSunShadowCascadeIndex(input.mViewWorldPos), input.mWorldPos, 0.0001, 1000.0)) 
		+ CalcLightsContribution(lightingData, ActiveLightsCount)) * input.mColor;
}

#endprogram
