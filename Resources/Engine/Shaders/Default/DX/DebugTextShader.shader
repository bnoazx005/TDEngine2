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

DECLARE_TEX2D(FontTextureAtlas)


float4 mainPS(VertexOut input): SV_TARGET0
{
	static const float2 params = float2(0.9, 20.0);

	float d = TEX2D(FontTextureAtlas, input.mUV).r;

	return float4(1.0, 1.0, 1.0, clamp((d-params.x)*params.y, 0.0, 1.0));
}

#endprogram
