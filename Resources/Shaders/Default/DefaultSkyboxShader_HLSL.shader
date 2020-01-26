#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>


struct VertexOut
{
	float4 mPos : SV_POSITION;
	float3 mUV  : TEXCOORD0;
};


#program vertex

struct VertexIn
{
	float4 mPos : POSITION0;
};


VertexOut mainVS(in VertexIn input)
{
	VertexOut output;

	float4x4 transformedView = ViewMat;
	transformedView._14_24_34 = 0.0f;

	output.mPos = mul(mul(ProjMat, transformedView), input.mPos).xyww;
	output.mUV  = input.mPos.xyz;

	return output;
}

#endprogram

#program pixel

// replace this declarations with DECLARE_TEXCUBE macro
TextureCube SkyboxTexture;

SamplerState SkyboxTexture_Sampler;


float4 mainPS(VertexOut input): SV_TARGET0
{	
	return SkyboxTexture.Sample(SkyboxTexture_Sampler, input.mUV);
}

#endprogram
