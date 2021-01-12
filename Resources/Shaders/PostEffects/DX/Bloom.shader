#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>

struct VertexOut
{
	float4 mPos   : SV_POSITION;
	float2 mUV    : TEXCOORD0;
};

#program vertex

VertexOut mainVS(uint id : SV_VertexID)
{
	VertexOut output;					
	
	float x = float((id & 1) << 2);
	float y = -float((id & 2) << 1);
	output.mPos = float4(x - 1, y + 1, 0, 1);
	output.mUV = float2(x, -y) * 0.5;

	return output;
}

#endprogram

#program pixel

DECLARE_TEX2D(FrameTexture);

CBUFFER_SECTION_EX(BloomParameters, 4)
	float threshold;
CBUFFER_ENDSECTION

float4 mainPS(VertexOut input): SV_TARGET0
{
	float4 color = TEX2D(FrameTexture, input.mUV);

	return lerp(float4(0.0, 0.0, 0.0, 0.0), color, dot(color, float4(0.2126, 0.7152, 0.0722, 1.0)) > threshold ? 1.0 : 0.0);
}
#endprogram