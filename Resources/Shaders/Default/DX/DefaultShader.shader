#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>


struct VertexOut
{
	float4 mPos   : SV_POSITION;
	float4 mLightPos : POSITION1;
	float4 mWorldPos : POSITION2;
	float4 mColor : COLOR;
	float2 mUV : TEXCOORD;
	float4 mNormal : NORMAL;
};


#program vertex

struct VertexIn
{
	float4 mPos   : POSITION0;
	float4 mColor : COLOR0;
	float2 mUV : TEXCOORD;
	float4 mNormal : NORMAL;
};


VertexOut mainVS(in VertexIn input)
{
	VertexOut output;

	output.mPos      = mul(mul(ProjMat, mul(ViewMat, ModelMat)), input.mPos);
	output.mLightPos = mul(mul(SunLightMat, ModelMat), input.mPos);
	output.mWorldPos = mul(ModelMat, input.mPos);
	output.mNormal   = mul(transpose(InvModelMat), input.mNormal);
	output.mUV       = input.mUV;
	output.mColor    = input.mColor;

	return output;
}

#endprogram

#program pixel

#include <TDEngine2ShadowMappingUtils.inc>


DECLARE_TEX2D(AlbedoMap);
DECLARE_TEX2D(NormalMap);
DECLARE_TEX2D(PropertiesMap);


float4 mainPS(VertexOut input): SV_TARGET0
{
	float4 viewPos = ViewMat._14_24_34_44;

	LightingData lightingData = CreateLightingData(input.mWorldPos, input.mNormal, 
												   normalize(viewPos - input.mWorldPos), 
												   GammaToLinear(TEX2D(AlbedoMap, input.mUV)),
												   GammaToLinear(TEX2D(PropertiesMap, input.mUV)));

	float4 sunLight = CalcSunLightContribution(CreateSunLight(SunLightPosition, SunLightDirection, float4(1.0, 1.0, 1.0, 1.0)), lightingData);

	float4 pointLightsContribution = float4(0.0, 0.0, 0.0, 0.0);

	for (int i = 0; i < ActivePointLightsCount; ++i)
	{
		pointLightsContribution += CalcPointLightContribution(PointLights[i], lightingData);
	}

	return (sunLight + pointLightsContribution)	* (1.0 - ComputeShadowFactorPCF(8, input.mLightPos, 0.0001, 1000.0)) * input.mColor;
}

#endprogram
