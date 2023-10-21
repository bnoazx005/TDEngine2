#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>

struct VertexOut
{
	float4 mPos : SV_POSITION;
	uint mID  : COLOR0;
};

#program vertex

VertexOut mainVS(float4 lPos : POSITION0)
{
	VertexOut output;			

	output.mPos = mul(ProjMat, mul(ViewMat, mul(ModelMat, lPos)));
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