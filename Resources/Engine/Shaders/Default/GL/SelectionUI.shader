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

out uint FragColor;

void main(void)
{
	FragColor = uint(ObjectID + uint(1));
}
#endprogram