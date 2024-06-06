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

out vec4 FragColor;

DECLARE_TEX2D(FrameTexture);

void main(void)
{
	FragColor = TEX2D(FrameTexture, VertOutUV);
}
#endprogram