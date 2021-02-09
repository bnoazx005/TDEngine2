#version 330 core

#include <TDEngine2Globals.inc>

#define VERTEX_ENTRY main
#define PIXEL_ENTRY main

CBUFFER_SECTION(UserData)
	float mValue;
	float mUserData2;
CBUFFER_ENDSECTION

#program vertex

layout (location = 0) in vec4 inlPos;
layout (location = 1) in vec4 inColor;

out vec4 VertOutColor;


void main(void)
{
	gl_Position = ProjMat * ViewMat * inlPos;

	VertOutColor = inColor;
}

#endprogram

#program pixel

in vec4 VertOutColor;

out vec4 FragColor;

void main(void)
{
	FragColor = VertOutColor;
}

#endprogram
