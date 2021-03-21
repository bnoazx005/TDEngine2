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

	output.mPos        = mul(mul(ProjMat, ViewMat), input.mPos);
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
	return TEX2D(MainTexture, input.mUV) * input.mColor;
}

#endprogram
