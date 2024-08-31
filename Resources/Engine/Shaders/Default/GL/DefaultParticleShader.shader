#version 330 core

#include <TDEngine2Globals.inc>
#include <TDEngine2ParticlesUtils.inc>

#define VERTEX_ENTRY main
#define PIXEL_ENTRY main

CBUFFER_SECTION_EX(Parameters, 4)
	uint  mIsTexturingEnabled;
	uint  mIsSoftParticlesEnabled;
	uint  mPadding;
	float mSmoothScale;
	float mContrastPower;
CBUFFER_ENDSECTION

DECLARE_TYPED_BUFFER_EX(TParticle, Particles, 0);
DECLARE_TYPED_BUFFER_EX(vec2, AliveParticlesIndexBuffer, 1);
DECLARE_TYPED_BUFFER_EX(uint, Counters, 2);

#program vertex

const vec4 BILLBOARD_QUAD_VERTICES[4] = vec4[4]
(
  vec4(-0.5f, 0.5f, 0.0f, 0.0f),
  vec4(0.5f, 0.5f, 1.0f, 0.0f),
  vec4(-0.5f, -0.5f, 0.0f, 1.0f),
  vec4(0.5f, -0.5f, 1.0f, 1.0f)
);

layout (location = 0) in vec4 inColor;
layout (location = 1) in vec4 inParticlePosAndSize;
layout (location = 2) in vec4 inParticleRotation;

out vec4 VertViewPos;
out vec2 VertOutUV;
out vec4 VertColor;
out int VertInstanceID;

	// uint vertIndex             : SV_VertexID;         						gl_VertexID & glVertexIndex
	// vec4 mColor              : COLOR0;
	// vec4 mParticlePosAndSize : TEXCOORD1;
	// vec4 mParticleRotation   : TEXCOORD2;
	// uint mInstanceId           : SV_InstanceID;                               gl_InstanceID & gl_InstanceIndex 

void main(void)
{
	float cosAngle = cos(inParticleRotation.x);
	float sinAngle = sin(inParticleRotation.x);

	mat3 rotZAxisMat = mat3(
		cosAngle, -sinAngle, 0.0f,
		sinAngle, cosAngle,  0.0f,
		0.0f,     0.0f,      1.0f);

	vec3 particleCenter;
	vec3 localPos;

	if (int(IsGPUParticlesEnabled) != 0)
	{
		int index = int(TYPED_BUFFER_ACCESS(AliveParticlesIndexBuffer)[TYPED_BUFFER_ACCESS(Counters)[0] - uint(gl_InstanceID + 1)].y);
		TParticle currParticleData = TYPED_BUFFER_ACCESS(Particles)[index];

		particleCenter = currParticleData.mPosition.xyz;
		localPos       = rotZAxisMat * (vec3(BILLBOARD_QUAD_VERTICES[gl_VertexID].xy, 0.0) * currParticleData.mSize);
		
		VertColor      = currParticleData.mColor;
	}
	else
	{
		particleCenter = (ModelMat * vec4(inParticlePosAndSize.xyz, 1.0)).xyz;
		localPos       = rotZAxisMat * (vec3(BILLBOARD_QUAD_VERTICES[gl_VertexID].xy, 0.0) * inParticlePosAndSize.w);

		VertColor      = inColor;
	}

	vec4 pos = ViewMat * vec4(particleCenter.xyz, 1.0) + vec4(localPos.xyz, 0.0);

	VertViewPos    = pos;
	gl_Position    = ProjMat * pos;
	VertOutUV      = BILLBOARD_QUAD_VERTICES[gl_VertexID].zw;
	VertInstanceID = gl_InstanceID;
}

#endprogram

#program pixel

DECLARE_TEX2D_EX(DepthTexture, 15);
DECLARE_TEX2D(MainTexture);


float ApplyContrast(float input, float contrastPower)
{
	bool isAboveHalf = input > 0.5;
	float result = 0.5 * pow(clamp(2.0 * (isAboveHalf ? 1.0 - input : input), 0.0, 1.0), contrastPower); 
	return isAboveHalf ? 1.0 - result : result;
}

in vec4 VertViewPos;
in vec2 VertUV;
in vec4 VertColor;
in int  VertInstanceID;

out vec4 FragColor;

void main(void)
{
	vec4 baseColor = VertColor;

	if (int(mIsTexturingEnabled) == 1)
	{
		baseColor *= GammaToLinear(TEX2D(MainTexture, VertUV));
	}

	if (int(mIsSoftParticlesEnabled) == 1)
	{
		float z = CameraProjectionParams.x * CameraProjectionParams.y / (CameraProjectionParams.y - texelFetch(DepthTexture, int2(gl_FragCoord.xy), 0).r * (CameraProjectionParams.y - CameraProjectionParams.x));
		float zdiff = (z - VertViewPos.z);

		float output = ApplyContrast(zdiff * mSmoothScale, mContrastPower);
		if (zdiff * output <= 1e-3f) 
			discard;

		baseColor *= output;
	}

	FragColor = baseColor;
}
#endprogram