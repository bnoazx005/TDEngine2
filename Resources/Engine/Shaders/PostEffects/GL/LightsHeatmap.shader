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
	ivec2 tileIndex = ivec2(floor(gl_FragCoord.xy / LIGHT_TILE_BLOCK_SIZE));

	const vec3 mapTex[6] = vec3[6]
	(
		vec3(0.0, 0.0, 0.0),
		vec3(0.0, 0.0, 1.0),
		vec3(0.0, 1.0, 1.0),
		vec3(0.0, 1.0, 0.0),
		vec3(1.0, 1.0, 0.0),
		vec3(1.0, 0.0, 0.0)
	);

	const int mapTexLen = 5;
	const int maxHeat = 50;

	float l = clamp(float(FETCH_TEX2D_LOD(LightGridTexture, tileIndex, 0).y) / maxHeat, 0.0, 1.0) * mapTexLen;
	vec4 heatmap = vec4(mix(mapTex[int(floor(l))], mapTex[int(ceil(l))], l - floor(l)), 0.8);

	FragColor = TEX2D(FrameTexture, VertOutUV) + heatmap;
}
#endprogram