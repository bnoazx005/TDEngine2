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

float4 mainVS(uint vertexId : SV_VertexID): SV_POSITION
{
	return mul(SunLightMat[0], mul(ModelMat, 
		ComputeSkinnedVertexPos(GetVertPos(vertexId, StartVertexOffset, StartIndexOffset), 
		GetVertJointWeights(vertexId, StartVertexOffset, StartIndexOffset), 
		GetVertJointIndices(vertexId, StartVertexOffset, StartIndexOffset))));
}

#endprogram

#program pixel

void mainPS(float4 wPos : SV_POSITION) {}
#endprogram