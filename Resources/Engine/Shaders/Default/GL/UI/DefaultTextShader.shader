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
	const float smoothing = 1.0 / 32.0;

	float d = TEX2D(Texture, TexCoords).r;
	float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, d);

	FragColor = vec4(VertColor.rgb, VertColor.a * alpha);
}

#endprogram
