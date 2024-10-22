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
#define TDE2_ENABLE_INDEX_BUFFER
#include <TDEngine2VertexFormats.inc>

VertexOutput mainVS(uint vertexId : SV_VertexID)
{
	VertexOutput output;
	output.mPos = mul(ProjMat, mul(ViewMat, mul(ModelMat, GetVertPos(vertexId, StartVertexOffset, StartIndexOffset))));
	
	return output;
}

#endprogram

#program pixel

void mainPS(VertexOutput input){}

#endprogram