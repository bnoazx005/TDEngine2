#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>

struct VertexOut
{
	float4 mPos   : SV_POSITION;
	float2 mUV    : TEXCOORD0;
	uint   mID    : COLOR0;
};

#program vertex

struct VertexIn
{
	float4 mPosUV : POSITION0;
};

VertexOut mainVS(VertexIn input)
{
	VertexOut output;
	
	output.mPos   = mul(ModelMat, float4(input.mPosUV.xy, 0.0, 1.0));
	output.mUV    = input.mPosUV.zw;
	output.mID    = ObjectID + 1;	

	return output;
}

#endprogram

#program pixel

uint mainPS(VertexOut input): SV_TARGET0
{
	return input.mID;
}
#endprogram