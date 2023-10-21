#version 330 core

#include <TDEngine2Globals.inc>

#define VERTEX_ENTRY main
#define PIXEL_ENTRY main

#program vertex

#include <TDEngine2SkinningUtils.inc>

layout (location = 0) in vec4 inlPos;
layout (location = 1) in vec4 inJointWeights;
layout (location = 2) in vec4 inJointIndices;

void main(void)
{
	gl_Position = ProjMat * ViewMat * ModelMat * ComputeSkinnedVertexPos(inlPos, inJointWeights, inJointIndices);
}

#endprogram

#program pixel

out vec4 FragColor;

void main(void)
{
	FragColor = vec4(1.0, 0.0, 1.0, abs(sin(4.0 * Time.x)));
}
#endprogram