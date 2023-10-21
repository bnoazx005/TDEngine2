#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>

#program vertex

#include <TDEngine2SkinningUtils.inc>

struct VertexIn
{
	float4 mPos     : POSITION0;
	float4 mJointWeights : BLENDWEIGHT; 
	uint4  mJointIndices : BLENDINDICES;
};

float4 mainVS(in VertexIn input): SV_POSITION
{
	return mul(ProjMat, mul(ViewMat, mul(ModelMat, ComputeSkinnedVertexPos(input.mPos, input.mJointWeights, input.mJointIndices))));
}

#endprogram

#program pixel

float4 mainPS(float4 wPos : SV_POSITION): SV_TARGET0
{
	return float4(1.0, 0.0, 1.0, abs(sin(4.0 * Time.x)));
}
#endprogram