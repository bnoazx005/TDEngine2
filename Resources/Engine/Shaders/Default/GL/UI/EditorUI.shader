#version 330 core

#include <TDEngine2Globals.inc>

#define VERTEX_ENTRY main
#define PIXEL_ENTRY main

#program vertex

layout (location = 0) in vec4 inPosUV;
layout (location = 1) in vec4 inColor;

out vec2 VertOutUV;
out vec4 VertOutColor;

void main(void)
{
	gl_Position  = ModelMat * vec4(inPosUV.xy, 0.0, 1.0);
	VertOutUV    = inPosUV.zw;
	VertOutColor = inColor;
}

#endprogram

#program pixel

CBUFFER_SECTION_EX(Parameters, 4)
	int mIsAlphaClipEnabled;
CBUFFER_ENDSECTION

in vec2 VertOutUV;
in vec4 VertOutColor;

out vec4 FragColor;

DECLARE_TEX2D(Texture);

void main(void)
{
	vec4 base = TEX2D(Texture, VertOutUV);
	
	if (mIsAlphaClipEnabled == 1)
	{
		if (base.a < 0.001)
		{
			discard;
		}
	}

	FragColor = VertOutColor * base;
}
#endprogram