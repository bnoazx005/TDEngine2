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

	const float3 mapTex[] = {
		float3(0,0,0),
		float3(0,0,1),
		float3(0,1,1),
		float3(0,1,0),
		float3(1,1,0),
		float3(1,0,0),
	};

	const uint mapTexLen = 5;
	const uint maxHeat = 50;

	float l = saturate((float)LightGridTexture[tileIndex].y / maxHeat) * mapTexLen;
	float4 heatmap = float4(lerp(mapTex[floor(l)], mapTex[ceil(l)], l - floor(l)), 0.8);

	return TEX2D(FrameTexture, input.mUV) + heatmap;
}
#endprogram