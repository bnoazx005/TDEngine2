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


static const uint MAX_SAMPLES_COUNT = 16;

static const float2 KernelSamples[MAX_SAMPLES_COUNT / 2 + 1] =
{
	float2(0.197417, 0.0),
	float2(0.17467, 1.0),
	float2(0.12098, 2.0),
	float2(0.065592, 3.0),
	float2(0.027835, 4.0),
	float2(0.009245, 6.0),
	float2(0.002403, 7.0),
	float2(0.000489, 8.0),
	float2(0.000078, 9.0),
};


CBUFFER_SECTION_EX(BlurParameters, 4)
	//float4 samples[MAX_SAMPLES_COUNT];
	float4 blurParams; // x - scale, y - angle, z is 1.0 / FrameTexture_width, w - 1.0 / FrameTexture_height
	uint samplesCount;
CBUFFER_ENDSECTION


float4 mainPS(VertexOut input): SV_TARGET0
{
	float4 color = TEX2D(FrameTexture, input.mUV) * KernelSamples[0].x;

	for (uint i = 1; i < (samplesCount / 2 + 1); ++i)
	{
		float2 currSample = KernelSamples[i];

		float2 r = blurParams.x * float2(currSample.y * blurParams.z * cos(blurParams.y), currSample.y * blurParams.w * sin(blurParams.y));

		color += currSample.x * TEX2D(FrameTexture, input.mUV + r);
		color += currSample.x * TEX2D(FrameTexture, input.mUV - r);
	}

	return color;
}

#endprogram