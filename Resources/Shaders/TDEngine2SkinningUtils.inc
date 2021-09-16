/*!
	\file TDEngine2SkinningUtils.inc
	\author Kasimov Ildar
	\date 22.07.2021

	\brief The file contains helper functions to simplify code related with animated meshes
*/

#ifndef TDENGINE2_SKINNING_UTILS_INC
#define TDENGINE2_SKINNING_UTILS_INC


CBUFFER_SECTION(SkinningData)
	float4x4 mJoints[MAX_JOINTS_COUNT];
	uint mUsedJointsCount;
CBUFFER_ENDSECTION


float4 ComputeSkinnedVertexPos(in float4 position, in float4 weights, in uint4 indices)
{	
	float weightsArray[MAX_VERTS_PER_JOINT];
	weightsArray[0] = weights.x;
	weightsArray[1] = weights.y;
	weightsArray[2] = weights.z;
	weightsArray[3] = weights.w;

	float4x4 jointMats[MAX_VERTS_PER_JOINT];
	jointMats[0] = mJoints[indices.x];
	jointMats[1] = mJoints[indices.y];
	jointMats[2] = mJoints[indices.z];
	jointMats[3] = mJoints[indices.w];

	float3 localPos = float3(0.0, 0.0, 0.0);

	for (int i = 0; i < MAX_VERTS_PER_JOINT; ++i)
	{
		localPos += (mul(jointMats[i], position) * weightsArray[i]).xyz;
	}

	return float4(localPos, 1.0);
}


#endif