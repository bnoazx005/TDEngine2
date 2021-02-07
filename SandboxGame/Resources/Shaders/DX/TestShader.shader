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
	float4 mPos        : POSITION0;
	float2 mUV         : TEXCOORD0;
	float4x4 mModelMat : TEXCOORD1;
	float4 mColor      : COLOR0;
    uint mInstanceId   : SV_InstanceID;
};


VertexOut mainVS(in VertexIn input)
{
	VertexOut output;

	output.mPos        = mul(ProjMat, mul(ViewMat, mul(input.mModelMat, input.mPos)));
	output.mUV         = input.mUV;
	output.mColor      = input.mColor;
	output.mInstanceId = input.mInstanceId;

	return output;
}

#endprogram

#program pixel

Texture2D TextureAtlas;
TextureCube SkyboxTexture;

SamplerState TexSampler;


float4 mainPS(VertexOut input): SV_TARGET0
{
	float4 c = TextureAtlas.Sample(TexSampler, input.mUV);
	
	float4 c2 = SkyboxTexture.Sample(TexSampler, float3(0, 0, 0));
	if (c.a < 0.1)
	{
		discard;
	}

	return c * input.mColor;
}

#endprogram
