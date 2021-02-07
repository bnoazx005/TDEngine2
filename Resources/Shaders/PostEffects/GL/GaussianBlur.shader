#version 330 core

#include <TDEngine2Globals.inc>

#define VERTEX_ENTRY main
#define PIXEL_ENTRY main

#program vertex

layout (location = 0) in vec4 inPosUV;

out vec2 VertOutUV;

void main(void)
{
	gl_Position = vec4(inPosUV.xy, 0.0f, 1.0);
	VertOutUV   = inPosUV.zw;
}

#endprogram

#program pixel

in vec2 VertOutUV;

out vec4 FragColor;


const int MAX_SAMPLES_COUNT = 16;

const vec2 KernelSamples[MAX_SAMPLES_COUNT / 2 + 1] = vec2[MAX_SAMPLES_COUNT / 2 + 1]
(
	vec2(0.197417, 0.0),
	vec2(0.17467, 1.0),
	vec2(0.12098, 2.0),
	vec2(0.065592, 3.0),
	vec2(0.027835, 4.0),
	vec2(0.009245, 6.0),
	vec2(0.002403, 7.0),
	vec2(0.000489, 8.0),
	vec2(0.000078, 9.0)
);


DECLARE_TEX2D(FrameTexture);

CBUFFER_SECTION_EX(BlurParameters, 4)
	//float4 samples[MAX_SAMPLES_COUNT];
	vec4 blurParams; // x - scale, y - angle, z is 1.0 / FrameTexture_width, w - 1.0 / FrameTexture_height
	int samplesCount;
CBUFFER_ENDSECTION

void main(void)
{
	vec4 color = TEX2D(FrameTexture, VertOutUV) * KernelSamples[0].x;

	for (int i = 1; i < (samplesCount / 2 + 1); ++i)
	{
		vec2 currSample = KernelSamples[i];

		vec2 r = blurParams.x * vec2(currSample.y * blurParams.z * cos(blurParams.y), currSample.y * blurParams.w * sin(blurParams.y));

		vec2 uv = VertOutUV + r;
		color += currSample.x * TEX2D(FrameTexture, uv);

		uv = VertOutUV - r;
		color += currSample.x * TEX2D(FrameTexture, uv);
	}

	FragColor = color;
}

#endprogram