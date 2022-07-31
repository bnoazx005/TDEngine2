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
	float4 mPos                : POSITION0;
	float2 mUV                 : TEXCOORD0;
	float4 mColor              : COLOR0;
	float4 mParticlePosAndSize : TEXCOORD1;
	float4 mParticleRotation   : TEXCOORD2;
    uint mInstanceId           : SV_InstanceID;
};


VertexOut mainVS(in VertexIn input)
{
	VertexOut output;

	float cosAngle = cos(input.mParticleRotation.x);
	float sinAngle = sin(input.mParticleRotation.x);

	float3x3 rotZAxisMat = float3x3(cosAngle, -sinAngle, 0.0f, sinAngle, cosAngle, 0.0f, 0.0f, 0.0f, 1.0f);

	float3 particleCenter = input.mParticlePosAndSize.xyz;
	float3 localPos       = mul(rotZAxisMat, input.mPos.xyz * input.mParticlePosAndSize.w);

	float4 pos = mul(ViewMat, float4(particleCenter.x, particleCenter.y, particleCenter.z, 1.0)) + float4(localPos.x, localPos.y, localPos.z, 0.0);

	output.mPos        = mul(ProjMat, pos);
	output.mUV         = input.mUV;
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
