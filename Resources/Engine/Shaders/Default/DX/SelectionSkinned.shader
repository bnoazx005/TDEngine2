#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>

struct VertexOut
{
	float4 mPos : SV_POSITION;
	uint mID  : COLOR0;
};

#program vertex

#include <TDEngine2SkinningUtils.inc>

struct VertexIn
{
	float4 mPos     : POSITION0;
	float4 mJointWeights : BLENDWEIGHT; 
	uint4  mJointIndices : BLENDINDICES;
};

VertexOut mainVS(in VertexIn input)
{
	VertexOut output;			

	output.mPos = mul(ProjMat, mul(ViewMat, mul(ModelMat, ComputeSkinnedVertexPos(input.mPos, input.mJointWeights, input.mJointIndices))));
	output.mID  = ObjectID + 1;										

	return output;
}

#endprogram

#program pixel

uint mainPS(VertexOut input): SV_TARGET0
{
	return input.mID;
}
#endprogram