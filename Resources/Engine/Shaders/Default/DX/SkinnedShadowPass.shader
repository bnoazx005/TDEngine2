#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>

#program vertex

#include <TDEngine2SkinningUtils.inc>

struct VertexIn
{
	float4 mPos          : POSITION0;
	float4 mJointWeights : BLENDWEIGHT; 
	uint4  mJointIndices : BLENDINDICES;
};

float4 mainVS(in VertexIn input): SV_POSITION
{
	return mul(SunLightMat[0], mul(ModelMat, ComputeSkinnedVertexPos(input.mPos, input.mJointWeights, input.mJointIndices)));
}

#endprogram

#program pixel

void mainPS(float4 wPos : SV_POSITION) {}
#endprogram