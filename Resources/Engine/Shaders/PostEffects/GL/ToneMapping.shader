#version 330 core

#include <TDEngine2Globals.inc>

#define VERTEX_ENTRY main
#define PIXEL_ENTRY main

#program vertex

layout (location = 0) in vec4 inPosUV;

out vec2 VertOutUV;

void main(void)
{
	gl_Position = vec4(inPosUV.xy, 0.0f, 1.0);
	VertOutUV   = inPosUV.zw;
}

#endprogram

#program pixel

in vec2 VertOutUV;

out vec4 FragColor;

DECLARE_TEX2D(FrameTexture);
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

void main(void)
{
	vec4 color = TEX2D(FrameTexture, VertOutUV);
	vec3 mappedColor = mix(color.rgb, 1 - exp(-color.rgb * toneMappingParams.y), toneMappingParams.x);
	
	FragColor = vec4(mix(mappedColor, ApplyGrading(mappedColor), colorGradingParams.x)/*LinearToGamma(mappedColor)*/, color.a); // disabled because of SRGB back buffer
}

#endprogram