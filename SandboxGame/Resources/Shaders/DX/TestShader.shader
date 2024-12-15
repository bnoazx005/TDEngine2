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

DECLARE_TEX2D_EX(TextureAtlas, 0);
DECLARE_CUBETEX_EX(SkyboxTexture, 1);

float4 mainPS(VertexOut input): SV_TARGET0
{
	return input.mColor;
}

#endprogram
