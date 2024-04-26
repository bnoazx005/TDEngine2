#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#define TILED_LIGHTING_ENABLED

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

DECLARE_TEX2D_EX(FrameTexture, 0);

float4 mainPS(VertexOut input): SV_TARGET0
{
	uint2 tileIndex = uint2(floor(input.mPos.xy / LIGHT_TILE_BLOCK_SIZE));
	uint lightsCount = LightGridTexture[tileIndex].y;

	float t = lightsCount / float(MAX_LIGHTS_PER_TILE);

	return TEX2D(FrameTexture, input.mUV) + float4(sin(t), sin(t * 2.0), cos(t), 0.0);
}
#endprogram