#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>
#include <TDEngine2ParticlesUtils.inc>


struct VertexOut
{
	float4 mPos      : SV_POSITION;
	float2 mUV       : TEXCOORD0;
	float4 mViewPos  : TEXCOORD1;
	float4 mColor    : COLOR;
	uint mInstanceId : SV_InstanceID;
};


CBUFFER_SECTION_EX(Parameters, 4)
	uint  mIsTexturingEnabled;
	uint  mIsSoftParticlesEnabled;
	uint  mPadding;
	float mSmoothScale;
	float mContrastPower;
CBUFFER_ENDSECTION

DECLARE_TYPED_BUFFER_EX(TParticle, Particles, 0);
DECLARE_TYPED_BUFFER_EX(float2, AliveParticlesIndexBuffer, 1);
DECLARE_TYPED_BUFFER_EX(uint, Counters, 2);

#program vertex

#define TDE2_USE_PARTICLE_VERTEX_FORMAT
#define TDE2_ENABLE_INDEX_BUFFER
#include <TDEngine2VertexFormats.inc>


static const float4 BILLBOARD_QUAD_VERTICES[] = 
{
  float4(-0.5f, 0.5f, 0.0f, 0.0f),
  float4(0.5f, 0.5f, 1.0f, 0.0f),
  float4(-0.5f, -0.5f, 0.0f, 1.0f),
  float4(0.5f, -0.5f, 1.0f, 1.0f),
};


VertexOut mainVS(uint vertexId : SV_VertexID, uint instanceId : SV_InstanceID)
{
	VertexOut output;

	float3 particleCenter;
	float3 localPos;

	float sinAngle, cosAngle;
	float3x3 rotZAxisMat;

	if (IsGPUParticlesEnabled)
	{
		uint index = (uint)AliveParticlesIndexBuffer[Counters[0] - 1 - instanceId].y;
		TParticle currParticleData = Particles[index];

		cosAngle = cos(currParticleData.mLifeParamsAndRotationPerFrame.z);
		sinAngle = sin(currParticleData.mLifeParamsAndRotationPerFrame.z);

		rotZAxisMat = float3x3(
				cosAngle, -sinAngle, 0.0f,
				sinAngle, cosAngle,  0.0f,
				0.0f,     0.0f,      1.0f);

		particleCenter = float4(currParticleData.mPositionAndSize.xyz, 1.0);
		localPos       = mul(rotZAxisMat, float3(BILLBOARD_QUAD_VERTICES[TYPED_BUFFER_ACCESS(IndexBuffer)[vertexId]].xy, 0.0) * currParticleData.mPositionAndSize.w);
		
		output.mColor = currParticleData.mColor;
	}
	else
	{
		float4 particleRotation = GetParticleRotation(instanceId);

		cosAngle = cos(particleRotation.x);
		sinAngle = sin(particleRotation.x);

		rotZAxisMat = float3x3(
				cosAngle, -sinAngle, 0.0f,
				sinAngle, cosAngle,  0.0f,
				0.0f,     0.0f,      1.0f);

		float4 particlePosAndSize = GetParticlePositionAndSize(instanceId);

		particleCenter = mul(ModelMat, float4(particlePosAndSize.xyz, 1.0));
		localPos       = mul(rotZAxisMat, float3(BILLBOARD_QUAD_VERTICES[TYPED_BUFFER_ACCESS(IndexBuffer)[vertexId]].xy, 0.0) * particlePosAndSize.w);

		output.mColor = GetParticleVertColor(instanceId);
	}

	float4 pos = mul(ViewMat, float4(particleCenter.x, particleCenter.y, particleCenter.z, 1.0)) + float4(localPos.x, localPos.y, localPos.z, 0.0);

	output.mViewPos    = pos;
	output.mPos        = mul(ProjMat, pos);
	output.mUV         = BILLBOARD_QUAD_VERTICES[TYPED_BUFFER_ACCESS(IndexBuffer)[vertexId]].zw;
	output.mInstanceId = instanceId;

	return output;
}

#endprogram

#program pixel

DECLARE_TEX2D_EX(DepthTexture, 15);
DECLARE_TEX2D(MainTexture);


float ApplyContrast(float input, float contrastPower)
{
	bool isAboveHalf = input > 0.5 ;
	float result = 0.5 * pow(saturate(2.0 * (isAboveHalf ? 1.0 - input : input)), contrastPower); 
	return isAboveHalf ? 1.0 - result : result;
}


float4 mainPS(VertexOut input): SV_TARGET0
{
	float4 baseColor = input.mColor;

	if (mIsTexturingEnabled)
	{
		baseColor *= GammaToLinear(TEX2D(MainTexture, input.mUV));
	}

	if (mIsSoftParticlesEnabled)
	{
		float z = CameraProjectionParams.x * CameraProjectionParams.y / (CameraProjectionParams.y - DepthTexture.Load(int4(input.mPos.xy, 0, 0)).r * (CameraProjectionParams.y - CameraProjectionParams.x));
		float zdiff = (z - input.mViewPos.z);

		float output = ApplyContrast(zdiff * mSmoothScale, mContrastPower);
		if (zdiff * output <= 1e-3f) 
			discard;

		return baseColor * output;
	}

	return baseColor;
}

#endprogram
