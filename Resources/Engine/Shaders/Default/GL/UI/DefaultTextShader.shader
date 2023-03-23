#version 330 core

#include <TDEngine2Globals.inc>

#define VERTEX_ENTRY main
#define PIXEL_ENTRY main


#program vertex

layout (location = 0) in vec4 inPosUV; // xy are screen-space coordinates, zw are uv coordinates
layout (location = 1) in vec4 inColor;

out vec2 TexCoords;
out vec4 VertColor;

void main(void)
{
	gl_Position = ModelMat * vec4(inPosUV.xy, 0.1, 1.0);// - vec4(1.5, 1.5, 0, 0);
	TexCoords = inPosUV.zw;

	VertColor = inColor;
}

#endprogram

#program pixel

DECLARE_TEX2D(Texture);

in vec2 TexCoords;
in vec4 VertColor;

out vec4 FragColor;


void main(void)
{	
	const float2 params = float2(0.9, 20.0);

	float d = TEX2D(Texture, TexCoords).r;

	FragColor = VertColor * float4(1.0, 1.0, 1.0, clamp((d-params.x)*params.y, 0.0, 1.0));
}

#endprogram
