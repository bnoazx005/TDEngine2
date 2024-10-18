#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>


struct VertexOut
{
	float4 mPos   : SV_POSITION;
	float4 mColor : COLOR;
};


#program vertex

#define TDE2_USE_UI_VERTEX_FORMAT
#include <TDEngine2VertexFormats.inc>

VertexOut mainVS(uint id : SV_VertexID)
{
	VertexOut output;

	output.mPos   = mul(mul(ProjMat, mul(ViewMat, ModelMat)), GetUiVertPosUv(id, StartVertexOffset, StartIndexOffset));
	output.mColor = GetUiVertColor(id, StartVertexOffset, StartIndexOffset);

	return output;
}

#endprogram

#program pixel

float4 mainPS(VertexOut input): SV_TARGET0
{
	return input.mColor;
}

#endprogram
