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

out vec4 FragColor;

void main(void)
{
	FragColor = vec4(1.0, 0.0, 1.0, abs(sin(4.0 * Time.x)));
}
#endprogram