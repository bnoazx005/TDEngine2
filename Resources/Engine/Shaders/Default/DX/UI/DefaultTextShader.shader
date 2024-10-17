#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>


struct VertexOut
{
	float4 mPos   : SV_POSITION;
	float2 mUV    : TEXCOORD0;
	float4 mColor : COLOR0;
};


#program vertex

#define TDE2_USE_UI_VERTEX_FORMAT
#include <TDEngine2VertexFormats.inc>

VertexOut mainVS(uint id : SV_VertexID)
{
	VertexOut output;

	float4 posUV = GetUiVertPosUv(id, StartVertexOffset, StartIndexOffset);

	output.mPos   = mul(ModelMat, float4(posUV.xy, 0.0, 1.0));
	output.mUV    = posUV.zw;
	output.mColor = GetUiVertColor(id, StartVertexOffset, StartIndexOffset);

	return output;
}

#endprogram

#program pixel

DECLARE_TEX2D(Texture)


float4 mainPS(VertexOut input): SV_TARGET0
{
	static const float smoothing = 1.0 / 32.0;

	float d = TEX2D(Texture, input.mUV).r;
	float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, d);

	return float4(input.mColor.rgb, input.mColor.a * alpha);
}

#endprogram
