/*!
	\file TDEngine2ParticlesUtils.inc
	\author Kasimov Ildar
	\date 05.07.2024

	\brief
*/

#ifndef TDENGINE2_PARTICLES_UTILS_INC
#define TDENGINE2_PARTICLES_UTILS_INC


#define BOX_EMITTER_TYPE_ID    0xfd9c40b2
#define SPHERE_EMITTER_TYPE_ID 0x264d34f0
#define CONE_EMITTER_TYPE_ID   0x2fd3124e

#define EMITTERS_CURVES_ATLAS_SIZE 1024
#define EMITTERS_CURVES_ATLAS_CURVE_WIDTH 128

// Particle flags
#define PARTICLE_FLAG_SIZE_OVER_LIFETIME_ENABLED     1
#define PARTICLE_FLAG_COLOR_OVER_LIFETIME_ENABLED    (1 << 1)
#define PARTICLE_FLAG_VELOCITY_OVER_LIFETIME_ENABLED (1 << 2)
#define PARTICLE_FLAG_ROTATION_OVER_LIFETIME_ENABLED (1 << 3)
#define PARTICLE_FLAG_GRAVITY_FORCE_ENABLED          (1 << 4)
#define PARTICLE_FLAG_FORCE_OVER_LIFETIME_ENABLED    (1 << 5)
#define PARTICLE_FLAG_USE_COLOR_RANDOM_TWEEN         (1 << 6)


struct TParticle
{
	float4 mPosition;
	float4 mVelocity;
	float4 mColor;
	float  mAge;
	float  mLifetime;
	float  mRotation;
	float  mSize;
	uint   mEmitterIndex;
	uint   mEmitterFlags;
	float  mGravityModifier;
	float  mRotationPerFrame;
	float3 mForcePerFrame;
	int2   mEmittersAtlasStartPos;
};


float4 GeneratePositionForBoxEmitter(float4 emitterPos, float4 shapeOrigin, float4 sizes, float4 rngValues)
{
	float4 minExtents = shapeOrigin - 0.5 * sizes;
	float4 maxExtents = shapeOrigin + 0.5 * sizes;

	// TODO: add support local/world sim spaces
	return emitterPos + float4(lerp(minExtents, maxExtents, rngValues).xyz, 1.0);
}


float4 GeneratePositionForSphereEmitter(float4 emitterPos, float4 shapeOrigin, float radius, float4 rngValues)
{
	// TODO: add support local/world sim spaces
	return emitterPos + shapeOrigin + float4(2.0 * rngValues.xyz - 1.0, 0.0) * radius;
}


float4 GeneratePositionForConeEmitter(float4 emitterPos, float4 shapeOrigin, float radius, float height, float4 rngValues)
{
	float2 pos = 2.0 * rngValues.xy - 1.0;
	float h = rngValues.z;

	float r = h * radius * 0.5 / height;
	pos = normalize(pos) * r;

	// TODO: add support local/world sim spaces
	return emitterPos + float4(pos.x, h, pos.y, 0.0);
}


#endif