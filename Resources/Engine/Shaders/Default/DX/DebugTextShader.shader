#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>


struct VertexOut
{
	float4 mPos : SV_POSITION;
	float2 mUV  : TEXCOORD0;
};


#program vertex

#define TDE2_USE_POSITION_ONLY_VERTEX_FORMAT
#define TDE2_FVF_POSITION_ONLY
#define TDE2_DECLARE_DEFAULT_VERTEX_BUFFER
#define TDE2_ENABLE_INDEX_BUFFER

#include <TDEngine2VertexFormats.inc>

VertexOut mainVS(uint id : SV_VertexID)
{
	VertexOut output;

	float4 posUV = GetVertPos(id, StartVertexOffset, StartIndexOffset);

	output.mPos = mul(ProjMat, float4(posUV.xy, 1.0, 1.0));
	output.mUV = posUV.zw;

	return output;
}

#endprogram

#program pixel

DECLARE_TEX2D(FontTextureAtlas)


float4 mainPS(VertexOut input): SV_TARGET0
{
	static const float2 params = float2(0.9, 20.0);

	float d = TEX2D(FontTextureAtlas, input.mUV).r;

	return float4(1.0, 1.0, 1.0, clamp((d-params.x)*params.y, 0.0, 1.0));
}

#endprogram
