#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>


struct VertexOut
{
	float4 mPos   : SV_POSITION;
	float4 mLightPos : POSITION1;
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

	output.mPos      = mul(mul(ProjMat, mul(ViewMat, ModelMat)), input.mPos);
	output.mLightPos = mul(mul(SunLightMat, ModelMat), input.mPos);
	output.mColor    = input.mColor;

	return output;
}

#endprogram

#program pixel

DECLARE_TEX2D(ShadowMapTexture)

float ComputeShadowFactor(Texture2D shadowMap, SamplerState sampleState, float4 lightSpaceFragPos, float bias)
{
	float3 projectedPos = lightSpaceFragPos.xyz / lightSpaceFragPos.w;

	if (projectedPos.z > 1.0)
	{
		return 0.0;
	}

	projectedPos.x = 0.5 * projectedPos.x + 0.5;
	projectedPos.y = -0.5 * projectedPos.y + 0.5;

	float sampledDepth = shadowMap.Sample(sampleState, projectedPos.xy);

	return ((projectedPos.z - bias) > sampledDepth ? 0.9 : 0.0);
}


float4 mainPS(VertexOut input): SV_TARGET0
{
	return (1.0 - ComputeShadowFactor(ShadowMapTexture, ShadowMapTexture_SamplerState, input.mLightPos, 0.0001)) * input.mColor;
}

#endprogram
