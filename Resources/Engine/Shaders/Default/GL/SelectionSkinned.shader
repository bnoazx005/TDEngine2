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

out uint FragColor;

void main(void)
{
	FragColor = uint(ObjectID + uint(1));
}
#endprogram