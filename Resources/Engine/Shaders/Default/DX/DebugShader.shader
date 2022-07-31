#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>


struct VertexOut
{
	float4 mPos   : SV_POSITION;
	float4 mColor : COLOR;
};


#program vertex

struct VertexIn
{
	float4 mPos   : POSITION0;
	float4 mColor : COLOR0;
};


VertexOut mainVS(in VertexIn input)
{
	VertexOut output;

	output.mPos   = mul(mul(ProjMat, mul(ViewMat, ModelMat)), input.mPos);
	output.mColor = input.mColor;

	return output;
}

#endprogram

#program pixel

float4 mainPS(VertexOut input): SV_TARGET0
{
	return input.mColor;
}

#endprogram
