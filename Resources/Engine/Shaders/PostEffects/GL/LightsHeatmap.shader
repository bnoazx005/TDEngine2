#version 330 core

#define TILED_LIGHTING_ENABLED

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

DECLARE_TEX2D_EX(FrameTexture, 0);

void main(void)
{
	ivec2 tileIndex = ivec2(floor(gl_FragPos.xy / LIGHT_TILE_BLOCK_SIZE));
	uint lightsCount = LightGridTexture[tileIndex].y;

	float t = lightsCount / float(MAX_LIGHTS_PER_TILE);

	FragColor = TEX2D(FrameTexture, VertOutUV) + float4(sin(t), sin(t * 2.0), cos(t), 0.0);
}
#endprogram