#version 330 core

#include <TDEngine2Globals.inc>

#define VERTEX_ENTRY main
#define PIXEL_ENTRY main

#program vertex

out vec2 VertOutUV;

void main(void)
{
	vec2 position = vec2(gl_VertexID >> 1, gl_VertexID & 1) * 4.0 - 1;

	gl_Position = vec4(position, 0.0f, 1.0);
	VertOutUV   = (position + 1) * 0.5;
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