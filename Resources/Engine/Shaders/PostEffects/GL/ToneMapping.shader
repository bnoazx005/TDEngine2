#version 330 core

#include <TDEngine2Globals.inc>

#define VERTEX_ENTRY main
#define PIXEL_ENTRY main

#program vertex

out vec2 VertOutUV;

void main(void)
{
    vec2 position = vec2(gl_VertexID & 2, gl_VertexID >> 1) * 4.0 - 1;

	gl_Position = vec4(position, 0.0f, 1.0);
	VertOutUV   = (position + 1) * 0.5;
}

#endprogram

#program pixel

in vec2 VertOutUV;

out vec4 FragColor;

DECLARE_TEX2D(FrameTexture);
DECLARE_TEX2D_EX(LuminanceBuffer, 1);
DECLARE_TEX2D_EX(UIBuffer, 2);
DECLARE_TEX2D(ColorGradingLUT);

CBUFFER_SECTION_EX(ToneMappingParameters, 4)
	vec4 toneMappingParams; // x  weight (0 is disabled, 1 is enabled), y - exposure
	vec4 colorGradingParams; // x - weight (enabled or not)
CBUFFER_ENDSECTION

// \todo Move this into TDEngine2EffectsUtils.inc later
vec3 ApplyGrading(in vec3 color)
{
	const float ColorsNum = 32.0;
	const float MaxColor = ColorsNum - 1.0;

	float cell = floor(color.b * MaxColor);

	float halfLUTxPixel = 0.5 / 1024.0; 
	float halfLUTyPixel = 0.5 / 32.0;

	const float threshold = MaxColor / ColorsNum;

	float xOffset = halfLUTxPixel + color.r / ColorsNum * threshold;
	float yOffset = halfLUTyPixel + color.g * threshold; 

	vec2 lutPos = vec2(cell / ColorsNum + xOffset, yOffset); 
	return TEX2D(ColorGradingLUT, lutPos).rgb;
}


const float MIDDLE_GRAY = 0.72f;
const float LUM_WHITE = 1.5f;


float GetLuminance() { return TEX2D(LuminanceBuffer, vec2(0.0, 0.0)).r; }


vec3 ReinhardToneMapping(vec3 color, float exposure)
{
	return 1 - exp(-color.rgb * exposure);
}


vec3 TestToneMapping(vec3 color)
{
	float lum = GetLuminance();

	color.rgb *= MIDDLE_GRAY / (lum + 0.001f);
    color.rgb *= (1.0f + color / LUM_WHITE);
    color.rgb /= (1.0f + color);

    return color;
}

/*
vec3 Reinhard2ToneMapping(vec3 color, float whiteBalance = 4.0)
{
	vec3 inLuminance = CalcLuminance(color);
	vec3 avgLuminance = GetLuminance();

	float lp = inLuminance / (9.6 * avgLuminance + 1e-4);

	float y = (lp * (1.0 + lp / (whiteBalance * whiteBalance))) / (1.0 + lp);

	return inLuminance;
}*/


vec3 FilmicToneMapping(vec3 color, float exposure)
{
	color *= exposure;

	color = max(vec3(0.0), color - 0.004f);
    return (color * (6.2f * color + 0.5f)) / (color * (6.2f * color + 1.7f)+ 0.06f);
}


void main(void)
{
	vec4 color = TEX2D(FrameTexture, VertOutUV);
	vec3 mappedColor = mix(color.rgb, ReinhardToneMapping(color.rgb, CalcExposure(GetLuminance(), 0.0, toneMappingParams.z)), toneMappingParams.x);
	
	mappedColor = mix(mappedColor, ApplyGrading(mappedColor), colorGradingParams.x);

	vec4 uiColor = TEX2D(UIBuffer, VertOutUV);

	FragColor = vec4(/*LinearToGamma*/clamp((1 - uiColor.a) * mappedColor + uiColor.rgb, 0.0, 1.0), max(color.a, uiColor.a)); // disabled because of SRGB back buffer

	//vec4 color = TEX2D(FrameTexture, VertOutUV);
	//vec3 mappedColor = mix(color.rgb, 1 - exp(-color.rgb * toneMappingParams.y), toneMappingParams.x);
	
	//FragColor = vec4(mix(mappedColor, ApplyGrading(mappedColor), colorGradingParams.x)/*LinearToGamma(mappedColor)*/, color.a); // disabled because of SRGB back buffer
}

#endprogram