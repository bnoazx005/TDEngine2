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

out float FragColor;

DECLARE_TEX2D(FrameTexture);
DECLARE_TEX2D(FrameTexture1);


CBUFFER_SECTION_EX(ShaderParameters, 4)
	float mAdaptationRate;
CBUFFER_ENDSECTION

void main(void)
{
	float currFrameAvgLuminance = exp(TEX2D_LOD(FrameTexture, vec2(0.5), 10.0).r);
	float prevLuminance = TEX2D(FrameTexture1, vec2(0.0)).r;

	// Adaptation based on Pattanaik's technique
	FragColor = prevLuminance + (currFrameAvgLuminance - prevLuminance) * (1.0 - exp(-Time.y * mAdaptationRate)); // Time.y is deltaTime
}

#endprogram