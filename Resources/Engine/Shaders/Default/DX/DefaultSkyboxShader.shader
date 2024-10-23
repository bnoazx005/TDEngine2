#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>


struct VertexOut
{
	float4 mPos : SV_POSITION;
	float3 mUV  : TEXCOORD0;
};


#program vertex

#define TDE2_DECLARE_DEFAULT_VERTEX_BUFFER
#define TDE2_ENABLE_INDEX_BUFFER
#include <TDEngine2VertexFormats.inc>


VertexOut mainVS(uint vertexId : SV_VertexID)
{
	VertexOut output;

	float4x4 transformedView = ViewMat;
	transformedView._14_24_34 = 0.0f;

	float4 pos = GetVertPos(vertexId, StartVertexOffset, StartIndexOffset);

	output.mPos = mul(mul(ProjMat, transformedView), pos).xyww;
	output.mUV  = pos.xyz;

	return output;
}

#endprogram

#program pixel

// replace this declarations with DECLARE_TEXCUBE macro
TextureCube SkyboxTexture;

SamplerState SkyboxTexture_Sampler;


float4 mainPS(VertexOut input): SV_TARGET0
{	
	return GammaToLinear(SkyboxTexture.Sample(SkyboxTexture_Sampler, input.mUV));
}

#endprogram
