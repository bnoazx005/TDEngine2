#version 330 core

#include <TDEngine2Globals.inc>

#define VERTEX_ENTRY main
#define PIXEL_ENTRY main


#program vertex

layout (location = 0) in vec4 inPos;

out vec3 VertOutUV;

void main(void)
{
	mat4 viewMat = ViewMat;
	vec4 pos = ProjMat * viewMat * inPos;

	gl_Position = pos.xyww;
	VertOutUV = inPos.xyz;
}

#endprogram

#program pixel

in vec3 VertOutUV;

out vec4 FragColor;

uniform samplerCube SkyboxTexture;

void main(void)
{
	FragColor = texture(SkyboxTexture, VertOutUV);
}

#endprogram
