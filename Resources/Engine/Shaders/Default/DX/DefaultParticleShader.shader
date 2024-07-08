#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>


struct VertexOut
{
	float4 mPos      : SV_POSITION;
	float2 mUV       : TEXCOORD0;
	float4 mColor    : COLOR;
    uint mInstanceId : SV_InstanceID;
};


#program vertex

struct VertexIn
{
	uint vertIndex             : SV_VertexID;
	float4 mColor              : COLOR0;
	float4 mParticlePosAndSize : TEXCOORD1;
	float4 mParticleRotation   : TEXCOORD2;
    uint mInstanceId           : SV_InstanceID;
};

static const float4 BILLBOARD_QUAD_VERTICES[] = 
{
  float4(-0.5f, 0.5f, 0.0f, 0.0f),
  float4(0.5f, 0.5f, 1.0f, 0.0f),
  float4(-0.5f, -0.5f, 0.0f, 1.0f),
  float4(0.5f, -0.5f, 1.0f, 1.0f),
};


VertexOut mainVS(in VertexIn input)
{
	VertexOut output;

	float cosAngle = cos(input.mParticleRotation.x);
	float sinAngle = sin(input.mParticleRotation.x);

	float3x3 rotZAxisMat = float3x3(cosAngle, -sinAngle, 0.0f, sinAngle, cosAngle, 0.0f, 0.0f, 0.0f, 1.0f);

	float3 particleCenter = mul(ModelMat, float4(input.mParticlePosAndSize.xyz, 1.0));
	float3 localPos       = mul(rotZAxisMat, float3(BILLBOARD_QUAD_VERTICES[input.vertIndex].xy, 0.0) * input.mParticlePosAndSize.w);

	float4 pos = mul(ViewMat, float4(particleCenter.x, particleCenter.y, particleCenter.z, 1.0)) + float4(localPos.x, localPos.y, localPos.z, 0.0);

	output.mPos        = mul(ProjMat, pos);
	output.mUV         = BILLBOARD_QUAD_VERTICES[input.vertIndex].zw;
	output.mColor      = input.mColor;
	output.mInstanceId = input.mInstanceId;

	return output;
}

#endprogram

#program pixel


DECLARE_TEX2D(MainTexture);


float4 mainPS(VertexOut input): SV_TARGET0
{
	return input.mColor;// TEX2D(MainTexture, input.mUV) * input.mColor;
}

#endprogram
