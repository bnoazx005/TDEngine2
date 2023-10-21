#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>

#program vertex

float4 mainVS(float4 lPos : POSITION0): SV_POSITION
{
	return mul(ProjMat, mul(ViewMat, mul(ModelMat, lPos)));
}

#endprogram

#program pixel

float4 mainPS(float4 wPos : SV_POSITION): SV_TARGET0
{
	return float4(1.0, 0.0, 1.0, abs(sin(4.0 * Time.x)));
}
#endprogram