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

DECLARE_TEX2D(FrameTexture); // depth buffer
DECLARE_TEX2D(FrameTexture1);

float4 mainPS(VertexOut input): SV_TARGET0
{    
    float4 cloudCol = TEX2D(FrameTexture1, input.mUV);

    if (TEX2D(FrameTexture, input.mUV).r < 0.999f)
        discard;
    
    return cloudCol;
}

#endprogram