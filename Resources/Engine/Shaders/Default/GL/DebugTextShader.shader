#version 330 core

#include <TDEngine2Globals.inc>

#define VERTEX_ENTRY main
#define PIXEL_ENTRY main


#program vertex

layout (location = 0) in vec4 inPosUV; // xy are screen-space coordinates, zw are uv coordinates

out vec2 TexCoords;

void main(void)
{
	gl_Position = ProjMat * ViewMat * vec4(inPosUV.xy, 0.1, 1.0);// - vec4(1.5, 1.5, 0, 0);
	TexCoords = inPosUV.zw;
}

#endprogram

#program pixel

DECLARE_TEX2D(FontTextureAtlas);

in vec2 TexCoords;

out vec4 FragColor;


void main(void)
{
	float distance = TEX2D(FontTextureAtlas, TexCoords).r;

	float dx = 1.0 / 512;

	distance += texture(FontTextureAtlas, TexCoords + vec2(-dx, 0.0)).r;
	distance += texture(FontTextureAtlas, TexCoords + vec2(dx, 0.0)).r;
	distance += texture(FontTextureAtlas, TexCoords + vec2(0.0, dx)).r;
	distance += texture(FontTextureAtlas, TexCoords + vec2(0.0, -dx)).r;

	distance /= 4;

    float a=min((distance-1.1)*1.4, 1.0);
	
	if (a < 0.1)
	{
		discard;
	}

	//FragColor = TexCoords.xyxy;
	FragColor = vec4(1.0);
}

#endprogram
