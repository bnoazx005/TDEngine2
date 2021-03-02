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
out vec4 VertOutWorldPos;
out vec2 VertOutUV;
out vec4 VertOutNormal;
out mat3 VertOutTBN;


void main(void)
{
	gl_Position = ProjMat * ViewMat * ModelMat * inlPos;
	LightSpaceVertPos = SunLightMat * ModelMat * inlPos;

	VertOutColor = inColor;

	VertOutWorldPos = ModelMat * inlPos;
	VertOutNormal = transpose(InvModelMat) * inNormal;
	VertOutUV = inUV;

	float3 tangent  = (transpose(InvModelMat) * inTangent).xyz;
	float3 binormal = cross(VertOutNormal.xyz, tangent);

	VertOutTBN = transpose(mat3(tangent, binormal, VertOutNormal.xyz));
}

#endprogram

#program pixel

#include <TDEngine2ShadowMappingUtils.inc>


DECLARE_TEX2D(AlbedoMap);
DECLARE_TEX2D(NormalMap);
DECLARE_TEX2D(PropertiesMap);


in vec4 VertOutColor;
in vec4 LightSpaceVertPos;
in vec4 VertOutWorldPos;
in vec2 VertOutUV;
in vec4 VertOutNormal;
in mat3 VertOutTBN;

out vec4 FragColor;

void main(void)
{
	vec3 normal = VertOutTBN * (2.0 * TEX2D(NormalMap, VertOutUV).xyz - 1.0);

	LightingData lightingData = CreateLightingData(VertOutWorldPos, vec4(normal, 0.0), 
												   normalize(CameraPosition - VertOutWorldPos), 
												   GammaToLinear(TEX2D(AlbedoMap, VertOutUV)),
												   TEX2D(PropertiesMap, VertOutUV));

	vec4 sunLight = CalcSunLightContribution(CreateSunLight(SunLightPosition, SunLightDirection, vec4(1.0)), lightingData);

	vec4 pointLightsContribution = vec4(0.0);

	for (int i = 0; i < ActivePointLightsCount; ++i)
	{
		pointLightsContribution += CalcPointLightContribution(PointLights[i], lightingData);
	}

	FragColor = (sunLight + pointLightsContribution) * (1.0 - ComputeShadowFactorPCF(8, LightSpaceVertPos, 0.0001, 1000.0)) * VertOutColor;
}

#endprogram
