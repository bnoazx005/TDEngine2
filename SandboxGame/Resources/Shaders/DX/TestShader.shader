#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>


struct VertexOut
{
	float4 mPos      : SV_POSITION;
	float2 mUV       : TEXCOORD0;
	float4 mColor    : COLOR;
    uint mInstanceId : SV_InstanceID;
};


#program vertex

#define TDE2_USE_SPRITE_VERTEX_FORMAT
#define TDE2_ENABLE_INDEX_BUFFER
#include <TDEngine2VertexFormats.inc>


VertexOut mainVS(uint vertexId : SV_VertexID, uint instanceId : SV_InstanceID)
{
	VertexOut output;

	output.mPos        = mul(ProjMat, mul(ViewMat, mul(GetSpriteTransform(instanceId), GetSpriteVertPos(vertexId))));
	output.mUV         = GetSpriteVertUv(vertexId);
	output.mColor      = GetSpriteVertColor(instanceId);
	output.mInstanceId = instanceId;

	return output;
}

#endprogram

#program pixel

Texture2D TextureAtlas;
TextureCube SkyboxTexture;

SamplerState TexSampler;


float4 mainPS(VertexOut input): SV_TARGET0
{
	float4 c = TextureAtlas.Sample(TexSampler, input.mUV);
	
	float4 c2 = SkyboxTexture.Sample(TexSampler, float3(0, 0, 0));
	if (c.a < 0.1)
	{
		discard;
	}

	return c * input.mColor;
}

#endprogram
