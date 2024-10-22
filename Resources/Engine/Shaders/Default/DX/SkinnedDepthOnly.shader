#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS
#define GEOMETRY_ENTRY mainGS

#include <TDEngine2Globals.inc>

struct VertexOutput
{
	float4 mPos : SV_POSITION;
};


#program vertex

#define TDE2_FVF_POSITION_ONLY
#define TDE2_DECLARE_DEFAULT_VERTEX_BUFFER
#define TDE2_FVF_SKINNED_VERTEX
#define TDE2_ENABLE_INDEX_BUFFER
#include <TDEngine2VertexFormats.inc>

#include <TDEngine2SkinningUtils.inc>

float4 mainVS(uint vertexId : SV_VertexID): SV_POSITION
{
	return mul(SunLightMat[0], mul(ModelMat, ComputeSkinnedVertexPos(GetVertPos(vertexId, StartVertexOffset, StartIndexOffset), 
		GetVertJointWeights(vertexId, StartVertexOffset, StartIndexOffset), 
		GetVertJointIndices(vertexId, StartVertexOffset, StartIndexOffset))));
}

#endprogram

#program pixel

void mainPS(VertexOutput input){}

#endprogram