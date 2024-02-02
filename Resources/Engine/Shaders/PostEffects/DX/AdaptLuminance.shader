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
DECLARE_TEX2D(FrameTexture1);


CBUFFER_SECTION_EX(ShaderParameters, 4)
	float mAdaptationRate;
CBUFFER_ENDSECTION


float mainPS(VertexOut input): SV_TARGET0
{
	float currFrameAvgLuminance = exp(TEX2D_LOD(FrameTexture, float2(0.5, 0.5), 10.0).r);
	float prevLuminance = TEX2D(FrameTexture1, float2(0.0, 0.0));

    // Adaptation based on Pattanaik's technique
   	return prevLuminance + (currFrameAvgLuminance - prevLuminance) * (1.0 - exp(-Time.y * mAdaptationRate)); // Time.y is deltaTime
}

#endprogram