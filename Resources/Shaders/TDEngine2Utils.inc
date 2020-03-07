/*!
	\file TDEngine2Globals.inc
	\author Kasimov Ildar
	\date 03.12.2018
*/

#ifndef TDENGINE2UTILS_INC
#define TDENGINE2UTILS_INC


///< Define constant buffer's macro

#ifdef TDE2_GLSL_SHADER

	#define CBUFFER_SECTION(name)			layout (std140) uniform name {
	#define CBUFFER_SECTION_EX(name, reg)	CBUFFER_SECTION(name)
	#define CBUFFER_ENDSECTION 				};
#endif

#ifdef TDE2_HLSL_SHADER

	#define CBUFFER_SECTION(name)			cbuffer name {
	#define CBUFFER_SECTION_EX(name, reg)	cbuffer name : register(b#reg) {
	#define CBUFFER_ENDSECTION 				};

#endif


///< Utility macro for defining samplers and their state objects
#ifdef TDE2_GLSL_SHADER

	#define DECLARE_TEX2D(SamplerName)	uniform sampler2D SamplerName
	#define TEX2D(SamplerName, uv)		texture(SamplerName, uv)

#endif

#ifdef TDE2_HLSL_SHADER

	#define DECLARE_TEX2D(SamplerName)					\
				Texture2D SamplerName;					\
				SamplerState SamplerName_SamplerState;

	#define TEX2D(SamplerName, uv) SamplerName.Sample(SamplerName_SamplerState, uv)

#endif


///< Types aliases (HLSL -> GLSL)

#ifdef TDE2_GLSL_SHADER
	
	#define float4x4 mat4
	#define float3x3 mat3

	#define float2 vec2
	#define float3 vec3
	#define float4 vec4

#endif


#include <TDEngine2EffectsUtils.inc>

#endif