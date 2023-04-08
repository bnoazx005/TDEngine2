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
out vec4 VertOutViewWorldPos;
out vec4 VertOutWorldPos;
out vec2 VertOutUV;
out vec4 VertOutNormal;
out mat3 VertOutTBN;
out vec3 OutTangentViewDir;


void main(void)
{
	gl_Position = ProjMat * ViewMat * ModelMat * inlPos;

	VertOutColor = inColor;

	VertOutWorldPos = ModelMat * inlPos;
	VertOutViewWorldPos = ViewMat * VertOutWorldPos;
	VertOutNormal = transpose(InvModelMat) * inNormal;
	VertOutUV = inUV;

	float3 tangent  = (transpose(InvModelMat) * inTangent).xyz;
	float3 binormal = normalize(cross(VertOutNormal.xyz, tangent));

	VertOutTBN = mat3(tangent, binormal, VertOutNormal.xyz);

	float3 view = CameraPosition.xyz - VertOutWorldPos.xyz;
	OutTangentViewDir = float3(dot(tangent, view), dot(view, binormal), dot(VertOutNormal.xyz, view));
}

#endprogram

#program pixel

#include <TDEngine2ShadowMappingUtils.inc>


DECLARE_TEX2D(AlbedoMap);
DECLARE_TEX2D(NormalMap);
DECLARE_TEX2D(PropertiesMap);


in vec4 VertOutColor;
in vec4 VertOutViewWorldPos;
in vec4 VertOutWorldPos;
in vec2 VertOutUV;
in vec4 VertOutNormal;
in mat3 VertOutTBN;
in vec3 OutTangentViewDir;

out vec4 FragColor;


TDE2_ENABLE_PARALLAX_MAPPING


CBUFFER_SECTION_EX(ShaderParameters, 4)
	float parallaxMappingEnabled;
	float4 tttt;
CBUFFER_ENDSECTION


void main(void)
{
	float2 uv = mix(VertOutUV, CalcParallaxMappingOffset(VertOutUV, normalize(OutTangentViewDir), normalize(VertOutNormal).xyz, 0.2, 8.0, 32.0), parallaxMappingEnabled);
	TDE2_DISCARD_PIXELS(uv);

	vec3 normal = VertOutTBN * (2.0 * TEX2D(NormalMap, uv).xyz - 1.0);

	LightingData lightingData = CreateLightingData(VertOutWorldPos, vec4(normal, 0.0), 
												   normalize(CameraPosition - VertOutWorldPos), 
												   GammaToLinear(TEX2D(AlbedoMap, uv)),
												   TEX2D(PropertiesMap, uv));

	vec4 sunLight = CalcSunLightContribution(CreateSunLight(SunLightPosition, SunLightDirection, vec4(1.0)), lightingData);

	vec4 pointLightsContribution = vec4(0.0);

	for (int i = 0; i < ActivePointLightsCount; ++i)
	{
		pointLightsContribution += CalcPointLightContribution(PointLights[i], lightingData) * (1.0 - ComputePointLightShadowFactor(PointLights[i], VertOutWorldPos, 0.1));
	}

	FragColor = (sunLight + pointLightsContribution) * (1.0 - ComputeSunShadowFactorPCF(8, GetSunShadowCascadeIndex(VertOutViewWorldPos), VertOutWorldPos, 0.0001, 1000.0)) * VertOutColor;
}

#endprogram
