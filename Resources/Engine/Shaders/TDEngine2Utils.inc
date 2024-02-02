/*!
	\file TDEngine2Utils.inc
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

	#define DECLARE_TEX2D(SamplerName)				 uniform sampler2D SamplerName
	#define DECLARE_TEX2D_EX(SamplerName, slot)		 uniform sampler2D SamplerName
	#define DECLARE_CUBETEX_EX(SamplerName, slot) 	 uniform samplerCube SamplerName
	#define DECLARE_TEX2DARRAY_EX(SamplerName, slot) uniform sampler2DArray SamplerName
	#define DECLARE_TEX3D_EX(SamplerName, slot) 	 uniform sampler3D SamplerName

	#define TEX2D(SamplerName, uv)			texture(SamplerName, uv)
	#define TEXCUBE(SamplerName, uv) 		TEX2D(SamplerName, uv)
	#define TEX3D(SamplerName, uv) 			TEX2D(SamplerName, uv)

	#define TEX2D_LOD(SamplerName, uv, lod)	textureLod(SamplerName, uv, lod)
	#define TEX3D_LOD(SamplerName, uv, lod) TEX2D_LOD(SamplerName, uv, lod)

	#define CONVERT_DEPTH_TO_NDC(depth) (2.0 * depth - 1.0)

#endif

#ifdef TDE2_HLSL_SHADER

	#define SAMPLER_NAME(SamplerName) 

	#define DECLARE_TEX2D(SamplerName)						\
				Texture2D SamplerName;			 			\
				SamplerState SamplerName ## _SamplerState;

	#define DECLARE_TEX2D_EX(SamplerName, slot)				\
				Texture2D SamplerName : register(t#slot); 	\
				SamplerState SamplerName ## _SamplerState;

	#define DECLARE_CUBETEX_EX(SamplerName, slot)			\
				TextureCube SamplerName : register(t#slot); \
				SamplerState SamplerName ## _SamplerState;

	#define DECLARE_TEX2DARRAY_EX(SamplerName, slot)			\
				Texture2DArray SamplerName : register(t#slot); 	\
				SamplerState SamplerName ## _SamplerState;

	#define DECLARE_TEX3D_EX(SamplerName, slot)				\
				Texture3D SamplerName : register(t#slot); 	\
				SamplerState SamplerName ## _SamplerState;

	#define TEX2D(SamplerName, uv) SamplerName.Sample(SamplerName ## _SamplerState, uv)
	#define TEXCUBE(SamplerName, uv) TEX2D(SamplerName, uv)
	#define TEX3D(SamplerName, uv) TEX2D(SamplerName, uv)

	#define TEX2D_LOD(SamplerName, uv, lod) SamplerName.SampleLevel(SamplerName ## _SamplerState, uv, lod)
	#define TEX3D_LOD(SamplerName, uv, lod) SamplerName.SampleLevel(SamplerName ## _SamplerState, uv, lod)

	#define CONVERT_DEPTH_TO_NDC(depth) (depth)

#endif


///< Types aliases (HLSL -> GLSL)

#ifdef TDE2_GLSL_SHADER
	
	#define float4x4 mat4
	#define float3x3 mat3

	#define float2 vec2
	#define float3 vec3
	#define float4 vec4
	#define uint4 uvec4

	#define mul(a, b) (a * b)

#endif


float LinearizeDepth(float depth, float zn, float zf) { return (2.0 * zn * zf) / (zn + zf - CONVERT_DEPTH_TO_NDC(depth) * (zf - zn)); }


/*!
	Global variables
*/

#define MAX_JOINTS_COUNT 256
#define MAX_VERTS_PER_JOINT 4

#ifdef TDE2_GLSL_SHADER

const float Pi = 3.141592;

#endif

#ifdef TDE2_HLSL_SHADER

static float Pi = 3.141592;

#endif


#include <TDEngine2EffectsUtils.inc>

#endif