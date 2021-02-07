#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>


struct VertexOut
{
	float4 mPos : SV_POSITION;
	float2 mUV  : TEXCOORD0;
};


#program vertex

struct VertexIn
{
	float4 mPos : POSITION0;
};


VertexOut mainVS(in VertexIn input)
{
	VertexOut output;

	output.mPos = mul(ProjMat, float4(input.mPos.xy, 1.0, 1.0));
	output.mUV = input.mPos.zw;

	return output;
}

#endprogram

#program pixel

Texture2D FontTextureAtlas;

SamplerState TexSampler;

float4 mainPS(VertexOut input): SV_TARGET0
{
	float4 d = FontTextureAtlas.Sample(TexSampler, input.mUV);
	
    float a=min((d-1.1)*1.4, 1.0);
	
	if (a < 0.1)
	{
		discard;
	}

	return float4(1, 1, 1, 1);
}

#endprogram
