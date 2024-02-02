#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>

struct VertexOut
{
	float4 mPos   : SV_POSITION;
	float2 mUV    : TEXCOORD0;
};

#program vertex

VertexOut mainVS(uint id : SV_VertexID)
{
	VertexOut output;					
	
	float x = float((id & 1) << 2);
	float y = -float((id & 2) << 1);
	output.mPos = float4(x - 1, y + 1, 0, 1);
	output.mUV = float2(x, -y) * 0.5;

	return output;
}

#endprogram

#program pixel

DECLARE_TEX2D(FrameTexture);
DECLARE_TEX2D_EX(LuminanceBuffer, 1);
DECLARE_TEX2D(ColorGradingLUT);

CBUFFER_SECTION_EX(ToneMappingParameters, 4)
	float4 toneMappingParams; // x  weight (0 is disabled, 1 is enabled), y - exposure, z - keyValue
	float4 colorGradingParams; // x - weight (enabled or not)
CBUFFER_ENDSECTION


// \todo Move this into TDEngine2EffectsUtils.inc later
float3 ApplyGrading(in float3 color)
{
	const float ColorsNum = 32.0;
	const float MaxColor = ColorsNum - 1.0;

	float cell = floor(color.b * MaxColor);

	float halfLUTxPixel = 0.5 / 1024.0; 
	float halfLUTyPixel = 0.5 / 32.0;

	const float threshold = MaxColor / ColorsNum;

	float xOffset = halfLUTxPixel + color.r / ColorsNum * threshold;
	float yOffset = halfLUTyPixel + color.g * threshold; 

	float2 lutPos = float2(cell / ColorsNum + xOffset, yOffset); 
	return TEX2D(ColorGradingLUT, lutPos).rgb;
}

static const float MIDDLE_GRAY = 0.72f;
static const float LUM_WHITE = 1.5f;


float GetLuminance() { return TEX2D(LuminanceBuffer, float2(0.0, 0.0)); }


float3 ReinhardToneMapping(float3 color, float exposure)
{
	return 1 - exp(-color.rgb * exposure);
}


float3 TestToneMapping(float3 color)
{
	float lum = GetLuminance();

	color.rgb *= MIDDLE_GRAY / (lum + 0.001f);
    color.rgb *= (1.0f + color / LUM_WHITE);
    color.rgb /= (1.0f + color);

    return color;
}


float3 Reinhard2ToneMapping(float3 color, float whiteBalance = 4.0)
{
	float3 inLuminance = CalcLuminance(color);
	float3 avgLuminance = GetLuminance();

	float lp = inLuminance / (9.6 * avgLuminance + 1e-4);

	float y = (lp * (1.0 + lp / (whiteBalance * whiteBalance))) / (1.0 + lp);

	return inLuminance;
}


float3 FilmicToneMapping(float3 color, float exposure)
{
	color *= exposure;

	color = max(0, color - 0.004f);
    return (color * (6.2f * color + 0.5f)) / (color * (6.2f * color + 1.7f)+ 0.06f);
}


float4 mainPS(VertexOut input): SV_TARGET0
{
	float4 color = TEX2D(FrameTexture, input.mUV);
	float3 mappedColor = lerp(color.rgb, FilmicToneMapping(color.rgb, CalcExposure(GetLuminance(), 0.0, toneMappingParams.z)), toneMappingParams.x);
	
	mappedColor = lerp(mappedColor, ApplyGrading(mappedColor), colorGradingParams.x);

	return float4(LinearToGamma(mappedColor), color.a);
}
#endprogram