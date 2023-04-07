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
out vec4 VertViewWorldPos;
out vec4 VertWorldPos;


void main(void)
{
	gl_Position = ProjMat * ViewMat * ModelMat * inlPos;
	VertWorldPos = ModelMat * inlPos;
	VertViewWorldPos = ViewMat * VertWorldPos;

	VertOutColor = inColor;
}

#endprogram

#program pixel

#include <TDEngine2ShadowMappingUtils.inc>

in vec4 VertOutColor;
in vec4 VertViewWorldPos;
in vec4 VertWorldPos;

out vec4 FragColor;

void main(void)
{
	FragColor = (1.0 - ComputeSunShadowFactorPCF(8, GetSunShadowCascadeIndex(VertViewWorldPos), VertWorldPos, 0.0001, 1000.0)) * VertOutColor;
}

#endprogram
