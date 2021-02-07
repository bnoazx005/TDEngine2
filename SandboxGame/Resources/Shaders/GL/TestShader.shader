#version 330 core

#include <TDEngine2Globals.inc>

#define VERTEX_ENTRY main
#define PIXEL_ENTRY main


#program vertex

layout (location = 0) in vec4 inlPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in mat4 inModelMat;
layout (location = 6) in vec4 inColor;

out vec2 VertOutUV;
out vec4 VertOutColor;

void main(void)
{
	gl_Position = ProjMat * ViewMat * inModelMat * inlPos;

	VertOutColor = inColor;

	VertOutUV = inUV;
}

#endprogram

#program pixel

in vec2 VertOutUV;
in vec4 VertOutColor;

out vec4 FragColor;

DECLARE_TEX2D(TextureAtlas);
uniform samplerCube SkyboxTexture;

void main(void)
{
	//FragColor = vec4(1);
	vec4 inColor = TEX2D(TextureAtlas, VertOutUV);
	//vec4 c2 = texture(SkyboxTexture, vec3(0));

	if (inColor.a < 0.1)
	{
		discard;
	}
	FragColor = inColor * VertOutColor;
	//FragColor = texture(TextureAtlas, VertOutUV).rrrr * VertOutColor;
	//FragColor = VertOutUV.xyxy;
}

#endprogram
