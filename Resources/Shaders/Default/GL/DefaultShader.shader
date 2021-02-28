#version 330 core

#include <TDEngine2Globals.inc>

#define VERTEX_ENTRY main
#define PIXEL_ENTRY main

#program vertex

layout (location = 0) in vec4 inlPos;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inNormal;
layout (location = 4) in vec4 inTangent;

out vec4 VertOutColor;
out vec4 LightSpaceVertPos;
out vec4 OutWorldPos;
out vec2 OutUV;
out vec4 OutNormal;
out mat3 OutTBN;


void main(void)
{
	gl_Position = ProjMat * ViewMat * ModelMat * inlPos;
	LightSpaceVertPos = SunLightMat * ModelMat * inlPos;

	VertOutColor = inColor;

	OutWorldPos = ModelMat * inlPos;
	OutNormal = transpose(InvModelMat) * inNormal;
	OutUV = inUV;

	float3 tangent  = (transpose(InvModelMat) * inTangent).xyz;
	float3 binormal = cross(OutNormal.xyz, tangent);

	OutTBN = transpose(mat3(tangent, binormal, OutNormal.xyz));
}

#endprogram

#program pixel

#include <TDEngine2ShadowMappingUtils.inc>


DECLARE_TEX2D(AlbedoMap);
DECLARE_TEX2D(NormalMap);
DECLARE_TEX2D(PropertiesMap);


in vec4 VertOutColor;
in vec4 LightSpaceVertPos;
in vec4 WorldPos;
in vec2 UV;
in vec4 Normal;
in mat3 TBN;

out vec4 FragColor;

void main(void)
{
	vec3 normal = TBN * (2.0 * TEX2D(NormalMap, UV).xyz - 1.0);

	LightingData lightingData = CreateLightingData(WorldPos, vec4(normal, 0.0), 
												   normalize(CameraPosition - WorldPos), 
												   GammaToLinear(TEX2D(AlbedoMap, UV)),
												   TEX2D(PropertiesMap, UV));

	vec4 sunLight = CalcSunLightContribution(CreateSunLight(SunLightPosition, SunLightDirection, vec4(1.0)), lightingData);

	vec4 pointLightsContribution = vec4(0.0);

	for (int i = 0; i < ActivePointLightsCount; ++i)
	{
		pointLightsContribution += CalcPointLightContribution(PointLights[i], lightingData);
	}

	//FragColor = (sunLight + pointLightsContribution) * (1.0 - ComputeShadowFactorPCF(8, LightSpaceVertPos, 0.0001, 1000.0)) * VertOutColor;
	FragColor = GammaToLinear(TEX2D(AlbedoMap, UV));
}

#endprogram
