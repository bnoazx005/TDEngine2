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

DECLARE_TEX2D(FrameTexture);

CBUFFER_SECTION_EX(BloomParameters, 4)
	float threshold;
CBUFFER_ENDSECTION

void main(void)
{
	vec4 color = TEX2D(FrameTexture, VertOutUV);
	FragColor = mix(vec4(0.0), color, dot(color, vec4(0.2126, 0.7152, 0.0722, 1.0)) > threshold ? 1.0 : 0.0);
}

#endprogram