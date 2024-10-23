#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>

#program vertex

#define TDE2_FVF_POSITION_ONLY
#define TDE2_DECLARE_DEFAULT_VERTEX_BUFFER
#define TDE2_FVF_SKINNED_VERTEX
#define TDE2_ENABLE_INDEX_BUFFER
#include <TDEngine2VertexFormats.inc>

#include <TDEngine2SkinningUtils.inc>

struct VertexIn
{
	float4 mPos     : POSITION0;
	float4 mJointWeights : BLENDWEIGHT; 
	uint4  mJointIndices : BLENDINDICES;
};

float4 mainVS(uint vertexId : SV_VertexID): SV_POSITION
{
	return mul(ProjMat, mul(ViewMat, mul(ModelMat, ComputeSkinnedVertexPos(GetVertPos(vertexId, StartVertexOffset, StartIndexOffset), 
		GetVertJointWeights(vertexId, StartVertexOffset, StartIndexOffset), 
		GetVertJointIndices(vertexId, StartVertexOffset, StartIndexOffset)))));
}

#endprogram

#program pixel

float4 mainPS(float4 wPos : SV_POSITION): SV_TARGET0
{
	return float4(1.0, 0.0, 1.0, abs(sin(4.0 * Time.x)));
}
#endprogram