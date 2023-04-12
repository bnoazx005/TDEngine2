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

struct VertexIn
{
	float4 mPosUV : POSITION0;
	float4 mColor : COLOR0;
};


VertexOut mainVS(in VertexIn input)
{
	VertexOut output;

	output.mPos   = mul(ModelMat, float4(input.mPosUV.xy, 0.0, 1.0));
	output.mUV    = input.mPosUV.zw;
	output.mColor = input.mColor;

	return output;
}

#endprogram

#program pixel

DECLARE_TEX2D(Texture)


float4 mainPS(VertexOut input): SV_TARGET0
{
	static const float2 params = float2(0.9, 20.0);

	static const float smoothing = 1.0 / 16.0;

	float d = TEX2D(Texture, input.mUV).r;
	float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, d);

	return float4(input.mColor.rgb, input.mColor.a * alpha);
}

#endprogram
