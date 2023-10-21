#version 330 core

#include <TDEngine2Globals.inc>

#define VERTEX_ENTRY main
#define PIXEL_ENTRY main

#program vertex

layout (location = 0) in vec4 inlPos;

void main(void)
{
	gl_Position = ProjMat * ViewMat * ModelMat * inlPos;
}

#endprogram

#program pixel

out uint FragColor;

void main(void)
{
	FragColor = uint(ObjectID + uint(1));
}
#endprogram