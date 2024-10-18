#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>

struct VertexOut
{
	float4 mPos   : SV_POSITION;
	float2 mUV    : TEXCOORD0;
	float4 mColor : COLOR;
};

#program vertex

#define TDE2_USE_UI_VERTEX_FORMAT
#define TDE2_ENABLE_INDEX_BUFFER
#include <TDEngine2VertexFormats.inc>

VertexOut mainVS(uint id : SV_VertexID)
{
	VertexOut output;
	
	float4 posUV = GetUiVertPosUv(id, StartVertexOffset, StartIndexOffset);
	
	output.mPos = mul(ModelMat, float4(posUV.xy, 0.0, 1.0));
	output.mUV = posUV.zw;
	output.mColor = GetUiVertColor(id, StartVertexOffset, StartIndexOffset);

	return output;
}

#endprogram

#program pixel

CBUFFER_SECTION_EX(Parameters, 4)
	int mIsAlphaClipEnabled;
CBUFFER_ENDSECTION

DECLARE_TEX2D(Texture);

float4 mainPS(VertexOut input): SV_TARGET0
{
	float4 base = TEX2D(Texture, MainTexture_TransformDesc.zw * (input.mUV + MainTexture_TransformDesc.xy));
	
	if (mIsAlphaClipEnabled == 1)
	{
		clip(base.a - 0.001);
	}

	return GammaToLinear(input.mColor * base); 
}
#endprogram