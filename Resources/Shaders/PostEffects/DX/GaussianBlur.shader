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

float4 mainPS(VertexOut input): SV_TARGET0
{
	float2 offsets[4] =
	{
		input.mUV + float2(-0.0025, 0.0),
		input.mUV + float2(0.0025, 0.0),
		input.mUV + float2(0.0, 0.0025),
		input.mUV + float2(0.0, -0.0025),
	};

	float4 color = TEX2D(FrameTexture, input.mUV);

	for (int i = 0; i < 4; ++i)
	{
		color += TEX2D(FrameTexture, offsets[i]);
	}

	return color * 0.2;
}
#endprogram