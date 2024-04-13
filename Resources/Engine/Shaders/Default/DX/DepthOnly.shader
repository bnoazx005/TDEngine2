#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS
#define GEOMETRY_ENTRY mainGS

#include <TDEngine2Globals.inc>

struct VertexOutput
{
	float4 mPos : SV_POSITION;
};


#program vertex

VertexOutput mainVS(float4 lPos : POSITION0)
{
	VertexOutput output;
	output.mPos = mul(ProjMat, mul(ViewMat, mul(ModelMat, lPos)));
	
	return output;
}

#endprogram

#program pixel

void mainPS(VertexOutput input){}

#endprogram