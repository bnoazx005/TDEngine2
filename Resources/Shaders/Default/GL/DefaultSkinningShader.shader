#version 330 core

#include <TDEngine2Globals.inc>

#define VERTEX_ENTRY main
#define PIXEL_ENTRY main

#program vertex

#include <TDEngine2SkinningUtils.inc>

layout (location = 0) in vec4 inlPos;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inNormal;
layout (location = 4) in vec4 inTangent;
layout (location = 5) in vec4  inJointWeights;
layout (location = 6) in uvec4 inJointIndices;

out vec4 VertOutColor;
out vec4 LightSpaceVertPos;
out vec4 VertOutWorldPos;
out vec2 VertOutUV;
out vec4 VertOutNormal;
out mat3 VertOutTBN;


void main(void)
{
	float weights[MAX_VERTS_PER_JOINT];
	weights[0] = inJointWeights.x;
	weights[1] = inJointWeights.y;
	weights[2] = inJointWeights.z;
	weights[3] = inJointWeights.w;

	mat4 jointMats[MAX_VERTS_PER_JOINT];
	jointMats[0] = mJoints[inJointIndices.x];
	jointMats[1] = mJoints[inJointIndices.y];
	jointMats[2] = mJoints[inJointIndices.z];
	jointMats[3] = mJoints[inJointIndices.w];

	vec3 localPos     = vec3(0.0);
	vec3 localNormal  = vec3(0.0);
	vec3 localTangent = vec3(0.0);

	for (int i = 0; i < MAX_VERTS_PER_JOINT; ++i)
	{
		localPos     += (mul(jointMats[i], inlPos) * weights[i]).xyz;
		localNormal  += (mul(jointMats[i], inNormal) * weights[i]).xyz;
		localTangent += (mul(jointMats[i], inTangent) * weights[i]).xyz;
	}

	vec4 pos = vec4(localPos, 1.0);

	gl_Position = ProjMat * ViewMat * ModelMat * pos;
	LightSpaceVertPos = SunLightMat * ModelMat * pos;

	VertOutColor = inColor;

	VertOutWorldPos = ModelMat * pos;
	VertOutNormal = transpose(InvModelMat) * vec4(localNormal, 0.0);
	VertOutUV = inUV;

	float3 tangent  = (transpose(InvModelMat) * vec4(localTangent, 0.0)).xyz;
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
	// \fixme Normal mapping doesn't look correctly
	vec3 normal = VertOutTBN * (2.0 * TEX2D(NormalMap, VertOutUV).xyz - 1.0);

	LightingData lightingData = CreateLightingData(VertOutWorldPos, vec4(VertOutNormal.xyz, 0.0), 
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
