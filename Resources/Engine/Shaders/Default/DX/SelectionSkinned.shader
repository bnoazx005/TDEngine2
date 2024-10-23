#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>

struct VertexOut
{
	float4 mPos : SV_POSITION;
	uint mID  : COLOR0;
};

#program vertex

#define TDE2_FVF_POSITION_ONLY
#define TDE2_DECLARE_DEFAULT_VERTEX_BUFFER
#define TDE2_FVF_SKINNED_VERTEX
#define TDE2_ENABLE_INDEX_BUFFER
#include <TDEngine2VertexFormats.inc>

#include <TDEngine2SkinningUtils.inc>


VertexOut mainVS(uint vertexId : SV_VertexID)
{
	VertexOut output;			

	output.mPos = mul(ProjMat, mul(ViewMat, mul(ModelMat, ComputeSkinnedVertexPos(GetVertPos(vertexId, StartVertexOffset, StartIndexOffset), 
		GetVertJointWeights(vertexId, StartVertexOffset, StartIndexOffset), 
		GetVertJointIndices(vertexId, StartVertexOffset, StartIndexOffset)))));
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