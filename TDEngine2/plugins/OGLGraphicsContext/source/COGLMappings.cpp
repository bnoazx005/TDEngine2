#include "./../include/COGLMappings.h"


namespace TDEngine2
{
	GLenum COGLMappings::GetUsageType(E_BUFFER_USAGE_TYPE type)
	{
		if (type == E_BUFFER_USAGE_TYPE::DYNAMIC)
		{
			return GL_DYNAMIC_DRAW;
		}

		return GL_STATIC_DRAW;
	}

	GLenum COGLMappings::GetBufferMapAccessType(E_BUFFER_MAP_TYPE type)
	{
		switch (type)
		{
			case BMT_READ:
				return GL_READ_ONLY;
			case BMT_READ_WRITE:
				return GL_READ_WRITE;
			case BMT_WRITE:
				return GL_WRITE_ONLY;
		}

		return GL_WRITE_ONLY;
	}
	
	GLint COGLMappings::GetInternalFormat(E_FORMAT_TYPE format)
	{
		switch (format)
		{
			case FT_FLOAT1:
			case FT_FLOAT1_TYPELESS:
				return GL_R32F;
			case FT_FLOAT2:
			case FT_FLOAT2_TYPELESS:
				return GL_RG32F;
			case FT_FLOAT3:
			case FT_FLOAT3_TYPELESS:
				return GL_RGB32F;
			case FT_FLOAT4:
			case FT_FLOAT4_TYPELESS:
				return GL_RGBA32F;
			case FT_NORM_BYTE1:
			case FT_BYTE1:
				return GL_R8_SNORM;
			case FT_NORM_BYTE2:
			case FT_BYTE2:
				return GL_RG8_SNORM;
			case FT_NORM_BYTE4:
			case FT_BYTE4:
				return GL_RGBA8_SNORM;
			case FT_NORM_UBYTE1:
			case FT_UBYTE1:
				return GL_R8;
			case FT_NORM_UBYTE2:
			case FT_UBYTE2:
				return GL_RG8;
			case FT_NORM_UBYTE4:
			case FT_UBYTE4:
			case FT_UBYTE4_BGRA_UNORM:
				return GL_RGBA8;
			case FT_NORM_SHORT1:
			case FT_SHORT1:
				return GL_R16_SNORM;
			case FT_NORM_SHORT2:
			case FT_SHORT2:
				return GL_RG16_SNORM;
			case FT_NORM_SHORT4:
			case FT_SHORT4:
				return GL_RGBA16_SNORM;
			case FT_NORM_USHORT1:
			case FT_USHORT1:
				return GL_R16;
			case FT_NORM_USHORT2:
			case FT_USHORT2:
				return GL_RG16;
			case FT_NORM_USHORT4:
			case FT_USHORT4:
				return GL_RGBA16;
			case FT_UINT1:
				return GL_R32UI;
			case FT_UINT2:
				return GL_RG32UI;
			case FT_UINT3:
				return GL_RGB32UI;
			case FT_UINT4:
				return GL_RGBA32UI;
			case FT_SINT1:
				return GL_R32I;
			case FT_SINT2:
				return GL_RGB32I;
			case FT_SINT3:
				return GL_RGB32I;
			case FT_SINT4:
				return GL_RGBA32I;
			case FT_D32:
				return GL_DEPTH_COMPONENT;
		}

		return 0;
	}

	GLenum COGLMappings::GetPixelDataFormat(E_FORMAT_TYPE format)
	{
		switch (format)
		{
			case FT_FLOAT1:
			case FT_FLOAT1_TYPELESS:
				return GL_RED;
			case FT_D32:
				return GL_DEPTH_COMPONENT;
			case FT_NORM_BYTE1:
			case FT_NORM_UBYTE1:
			case FT_NORM_SHORT1:
			case FT_NORM_USHORT1:
				return GL_RED;
			case FT_BYTE1:
			case FT_UBYTE1:
			case FT_SHORT1:
			case FT_USHORT1:
			case FT_UINT1:
			case FT_SINT1:
				return GL_RED_INTEGER;
			case FT_NORM_BYTE2:
			case FT_BYTE2:
			case FT_FLOAT2:
			case FT_FLOAT2_TYPELESS:
			case FT_NORM_UBYTE2:
			case FT_UBYTE2:
			case FT_NORM_SHORT2:
			case FT_SHORT2:
			case FT_NORM_USHORT2:
			case FT_USHORT2:
			case FT_UINT2:
			case FT_SINT2:
				return GL_RG;
			case FT_FLOAT3:
			case FT_FLOAT3_TYPELESS:
			case FT_UINT3:
			case FT_SINT3:
				return GL_RGB;
			case FT_FLOAT4:
			case FT_FLOAT4_TYPELESS:
			case FT_NORM_UBYTE4:
			case FT_UBYTE4:
			case FT_NORM_BYTE4:
			case FT_BYTE4:
			case FT_NORM_SHORT4:
			case FT_SHORT4:
			case FT_NORM_USHORT4:
			case FT_USHORT4:
			case FT_UINT4:
			case FT_SINT4:
				return GL_RGBA;
			case FT_UBYTE4_BGRA_UNORM:
				return  GL_BGRA;
		}

		return GL_DEPTH_STENCIL;
	}

	GLenum COGLMappings::GetWritableTextureFormat(E_FORMAT_TYPE format)
	{
		switch (format)
		{
			case FT_FLOAT4:
			case FT_FLOAT4_TYPELESS:
				return GL_RGBA32F;
		}

		return GL_RGBA8;
	}

	GLenum COGLMappings::GetShaderStageType(E_SHADER_STAGE_TYPE shaderStageType)
	{
		switch (shaderStageType)
		{
			case SST_VERTEX:
				return GL_VERTEX_SHADER;
			case SST_PIXEL:
				return GL_FRAGMENT_SHADER;
			case SST_GEOMETRY:
				return GL_GEOMETRY_SHADER;
			case SST_COMPUTE:
				return GL_COMPUTE_SHADER;
		}

		return 0;
	}

	U32 COGLMappings::GetNumOfChannelsOfFormat(E_FORMAT_TYPE format)
	{
		switch (format)
		{
			case FT_BYTE1:
			case FT_NORM_BYTE1:
			case FT_UBYTE1:
			case FT_NORM_UBYTE1:
			case FT_FLOAT1:
			case FT_D32:
			case FT_SHORT1:
			case FT_NORM_SHORT1:
			case FT_USHORT1:
			case FT_NORM_USHORT1:
			case FT_UINT1:
			case FT_NORM_UINT1:
			case FT_SINT1:
			case FT_NORM_SINT1:
				return 1;
			case FT_BYTE2:
			case FT_NORM_BYTE2:
			case FT_UBYTE2:
			case FT_NORM_UBYTE2:
			case FT_FLOAT2:
			case FT_FLOAT2_TYPELESS:
			case FT_SHORT2:
			case FT_NORM_SHORT2:
			case FT_USHORT2:
			case FT_NORM_USHORT2:
			case FT_UINT2:
			case FT_NORM_UINT2:
			case FT_SINT2:
			case FT_NORM_SINT2:
				return 2;
			case FT_BYTE3:
			case FT_NORM_BYTE3:
			case FT_UBYTE3:
			case FT_NORM_UBYTE3:
			case FT_FLOAT3:
			case FT_FLOAT3_TYPELESS:
			case FT_SHORT3:
			case FT_NORM_SHORT3:
			case FT_USHORT3:
			case FT_NORM_USHORT3:
			case FT_UINT3:
			case FT_NORM_UINT3:
			case FT_SINT3:
			case FT_NORM_SINT3:
				return 3;
			case FT_BYTE4:
			case FT_NORM_BYTE4:
			case FT_UBYTE4:
			case FT_NORM_UBYTE4:
			case FT_NORM_BYTE4_SRGB:
			case FT_UBYTE4_BGRA_UNORM:
			case FT_SHORT4:
			case FT_NORM_SHORT4:
			case FT_SINT4:
			case FT_NORM_SINT4:
			case FT_UINT4:
			case FT_NORM_UINT4:
			case FT_USHORT4:
			case FT_NORM_USHORT4:
			case FT_FLOAT4:
			case FT_FLOAT4_TYPELESS:
				return 4;
		}

		return 0;
	}

	GLenum COGLMappings::GetBaseTypeOfFormat(E_FORMAT_TYPE format)
	{
		switch (format)
		{
			case FT_NORM_BYTE1:
			case FT_BYTE1:
			case FT_NORM_BYTE2:
			case FT_BYTE2:
			case FT_NORM_BYTE4:
			case FT_BYTE4:
				return GL_BYTE;
			case FT_NORM_UBYTE1:
			case FT_UBYTE1:
			case FT_NORM_UBYTE2:
			case FT_UBYTE2:
			case FT_NORM_UBYTE4:
			case FT_UBYTE4:
			case FT_UBYTE4_BGRA_UNORM:
				return GL_UNSIGNED_BYTE;
			case FT_NORM_SHORT1:
			case FT_SHORT1:
			case FT_NORM_SHORT2:
			case FT_SHORT2:
			case FT_NORM_SHORT4:
			case FT_SHORT4:
				return GL_SHORT;
			case FT_NORM_USHORT1:
			case FT_USHORT1:
			case FT_NORM_USHORT2:
			case FT_USHORT2:
			case FT_NORM_USHORT4:
			case FT_USHORT4:
				return GL_UNSIGNED_SHORT;
			case FT_UINT1:
			case FT_UINT2:
			case FT_UINT3:
			case FT_UINT4:
				return GL_UNSIGNED_INT;
			case FT_SINT1:
			case FT_SINT2:
			case FT_SINT3:
			case FT_SINT4:
				return GL_INT;
		}

		return GL_FLOAT;
	}

	GLsizei COGLMappings::GetFormatSize(E_FORMAT_TYPE format)
	{
		switch (format)
		{
			case FT_BYTE1:
			case FT_NORM_BYTE1:
				return sizeof(char);
			case FT_BYTE2:
			case FT_NORM_BYTE2:
				return 2 * sizeof(char);
			case FT_BYTE3:
			case FT_NORM_BYTE3:
				return 3 * sizeof(char);
			case FT_BYTE4:
			case FT_NORM_BYTE4:
				return 4 * sizeof(char);
			case FT_UBYTE1:
			case FT_NORM_UBYTE1:
				return sizeof(unsigned char);
			case FT_UBYTE2:
			case FT_NORM_UBYTE2:
				return 2 * sizeof(unsigned char);
			case FT_UBYTE3:
			case FT_NORM_UBYTE3:
				return 3 * sizeof(unsigned char);
			case FT_UBYTE4:
			case FT_NORM_UBYTE4:
			case FT_NORM_BYTE4_SRGB:
			case FT_UBYTE4_BGRA_UNORM:
				return 4 * sizeof(unsigned char);
			case FT_FLOAT1:
			case FT_D32:
				return sizeof(float);
			case FT_FLOAT2:
			case FT_FLOAT2_TYPELESS:
				return 2 * sizeof(float);
			case FT_FLOAT3:
			case FT_FLOAT3_TYPELESS:
				return 3 * sizeof(float);
			case FT_FLOAT4:
			case FT_FLOAT4_TYPELESS:
				return 4 * sizeof(float);
			case FT_SHORT1:
			case FT_NORM_SHORT1:
				return 1 * sizeof(short);
			case FT_SHORT2:
			case FT_NORM_SHORT2:
				return 2 * sizeof(short);
			case FT_SHORT3:
			case FT_NORM_SHORT3:
				return 3 * sizeof(short);
			case FT_SHORT4:
			case FT_NORM_SHORT4:
				return 4 * sizeof(short);
			case FT_USHORT1:
			case FT_NORM_USHORT1:
				return 1 * sizeof(short);
			case FT_USHORT2:
			case FT_NORM_USHORT2:
				return 2 * sizeof(short);
			case FT_USHORT3:
			case FT_NORM_USHORT3:
				return 3 * sizeof(short);
			case FT_USHORT4:
			case FT_NORM_USHORT4:
				return 4 * sizeof(short);
			case FT_UINT1:
			case FT_NORM_UINT1:
				return 1 * sizeof(int);
			case FT_UINT2:
			case FT_NORM_UINT2:
				return 2 * sizeof(int);
			case FT_UINT3:
			case FT_NORM_UINT3:
				return 3 * sizeof(int);
			case FT_UINT4:
			case FT_NORM_UINT4:
				return 4 * sizeof(int);
			case FT_SINT1:
			case FT_NORM_SINT1:
				return 1 * sizeof(int);
			case FT_SINT2:
			case FT_NORM_SINT2:
				return 2 * sizeof(int);
			case FT_SINT3:
			case FT_NORM_SINT3:
				return 3 * sizeof(int);
			case FT_SINT4:
			case FT_NORM_SINT4:
				return 4 * sizeof(int);
		}

		return 0;
	}

	GLint COGLMappings::GetTypeSize(GLenum type)
	{
		switch (type)
		{
			case GL_FLOAT:
				return sizeof(F32);
			case GL_FLOAT_VEC2:
				return sizeof(F32) * 2;
			case GL_FLOAT_VEC3:
				return sizeof(F32) * 3;
			case GL_FLOAT_VEC4:
				return sizeof(F32) * 4;
			case GL_DOUBLE:
				return sizeof(F64);
			case GL_DOUBLE_VEC2:
				return sizeof(F64) * 2;
			case GL_DOUBLE_VEC3:
				return sizeof(F64) * 3;
			case GL_DOUBLE_VEC4:
				return sizeof(F64) * 4;
			case GL_INT:
				return sizeof(I32);
			case GL_INT_VEC2:
				return sizeof(I32) * 2;
			case GL_INT_VEC3:
				return sizeof(I32) * 3;
			case GL_INT_VEC4:
				return sizeof(I32) * 4;
			case GL_UNSIGNED_INT:
				return sizeof(U32);
			case GL_UNSIGNED_INT_VEC2:
				return sizeof(U32) * 2;
			case GL_UNSIGNED_INT_VEC3:
				return sizeof(U32) * 3;
			case GL_UNSIGNED_INT_VEC4:
				return sizeof(U32) * 4;
			case GL_BOOL:
				return sizeof(bool);
			case GL_BOOL_VEC2:
				return sizeof(bool) * 2;
			case GL_BOOL_VEC3:
				return sizeof(bool) * 3;
			case GL_BOOL_VEC4:
				return sizeof(bool) * 4;
			case GL_FLOAT_MAT2:
				return sizeof(F32) * 4;
			case GL_FLOAT_MAT3:
				return sizeof(F32) * 9;
			case GL_FLOAT_MAT4:
				return sizeof(F32) * 16;
			case GL_FLOAT_MAT2x3:
			case GL_FLOAT_MAT3x2:
				return sizeof(F32) * 6;
			case GL_FLOAT_MAT4x2:
			case GL_FLOAT_MAT2x4:
				return sizeof(F32) * 8;
			case GL_FLOAT_MAT4x3:
			case GL_FLOAT_MAT3x4:
				return sizeof(F32) * 12;
			case GL_DOUBLE_MAT2:
				return sizeof(F64) * 9;
			case GL_DOUBLE_MAT3:
				return sizeof(F64) * 9;
			case GL_DOUBLE_MAT4:
				return sizeof(F64) * 9;
			case GL_DOUBLE_MAT2x3:
			case GL_DOUBLE_MAT3x2:
				return sizeof(F64) * 6;
			case GL_DOUBLE_MAT2x4:
			case GL_DOUBLE_MAT4x2:
				return sizeof(F64) * 8;
			case GL_DOUBLE_MAT3x4:
			case GL_DOUBLE_MAT4x3:
				return sizeof(F64) * 12;
		}

		TDE2_UNREACHABLE();

		return 0;
	}

	bool COGLMappings::IsFormatNormalized(E_FORMAT_TYPE format)
	{
		switch (format)
		{
			case FT_NORM_BYTE1:
			case FT_NORM_BYTE2:
			case FT_NORM_BYTE4:
			case FT_NORM_UBYTE1:
			case FT_NORM_UBYTE2:
			case FT_NORM_UBYTE4:
			case FT_UBYTE4_BGRA_UNORM:
			case FT_NORM_SHORT1:
			case FT_NORM_SHORT2:
			case FT_NORM_SHORT4:
			case FT_NORM_USHORT1:
			case FT_NORM_USHORT2:
			case FT_NORM_USHORT4:
			case FT_D32:
				return true;
		}

		return false;
	}

	GLenum COGLMappings::GetPrimitiveTopology(E_PRIMITIVE_TOPOLOGY_TYPE topologyType)
	{
		switch (topologyType)
		{
			case E_PRIMITIVE_TOPOLOGY_TYPE::PTT_POINT_LIST:
				return GL_POINTS;
			case E_PRIMITIVE_TOPOLOGY_TYPE::PTT_LINE_LIST:
				return GL_LINES;
			case E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST:
				return GL_TRIANGLES;
			case E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_STRIP:
				return GL_TRIANGLE_STRIP;
			case E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_FAN:
				return GL_TRIANGLE_FAN;
		}

		return GL_TRIANGLES;
	}

	GLenum COGLMappings::GetIndexFormat(E_INDEX_FORMAT_TYPE indexFormatType)
	{
		switch (indexFormatType)
		{
			case E_INDEX_FORMAT_TYPE::INDEX16:
				return GL_UNSIGNED_SHORT;
			case E_INDEX_FORMAT_TYPE::INDEX32:
				return GL_UNSIGNED_INT;
		}

		return 0;
	}

	GLint COGLMappings::GetMinFilterType(E_TEXTURE_FILTER_TYPE filterValue, bool useMipMaps)
	{
		switch (filterValue)
		{
			case E_TEXTURE_FILTER_TYPE::FT_POINT:
				return useMipMaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;

			case E_TEXTURE_FILTER_TYPE::FT_BILINEAR:
				return useMipMaps ? GL_NEAREST_MIPMAP_LINEAR : GL_LINEAR;

			case E_TEXTURE_FILTER_TYPE::FT_TRILINEAR:
				return GL_LINEAR_MIPMAP_LINEAR;

			case E_TEXTURE_FILTER_TYPE::FT_ANISOTROPIC:
				return GL_LINEAR_MIPMAP_LINEAR;
		}

		return GL_NEAREST;
	}

	GLint COGLMappings::GetMagFilterType(E_TEXTURE_FILTER_TYPE filterValue)
	{
		switch (filterValue)
		{
			case E_TEXTURE_FILTER_TYPE::FT_POINT:
				return GL_NEAREST;

			case E_TEXTURE_FILTER_TYPE::FT_BILINEAR:
			case E_TEXTURE_FILTER_TYPE::FT_TRILINEAR:
				return GL_LINEAR;

			case E_TEXTURE_FILTER_TYPE::FT_ANISOTROPIC:
				TDE2_UNIMPLEMENTED();
				return 0;
		}

		return GL_NEAREST;
	}

	GLint COGLMappings::GetTextureAddressMode(E_ADDRESS_MODE_TYPE addressMode)
	{
		switch (addressMode)
		{
			case E_ADDRESS_MODE_TYPE::AMT_BORDER:
				return GL_CLAMP_TO_BORDER;
			case E_ADDRESS_MODE_TYPE::AMT_CLAMP:
				return GL_CLAMP_TO_EDGE;
			case E_ADDRESS_MODE_TYPE::AMT_MIRROR:
				return GL_MIRRORED_REPEAT;
			case E_ADDRESS_MODE_TYPE::AMT_WRAP:
				return GL_REPEAT;
		}

		return GL_REPEAT;
	}

	E_RESULT_CODE COGLMappings::GetErrorCode(GLenum error)
	{
		switch (error)
		{
			case GL_INVALID_ENUM:
				return RC_INVALID_ARGS;
			case GL_INVALID_VALUE:
				return RC_INVALID_ARGS;
			case GL_OUT_OF_MEMORY:
				return RC_OUT_OF_MEMORY;
			case GL_INVALID_OPERATION:
			case GL_STACK_OVERFLOW:
			case GL_STACK_UNDERFLOW:
			case GL_INVALID_FRAMEBUFFER_OPERATION:
			case GL_CONTEXT_LOST:
				return RC_FAIL;
			case GL_NO_ERROR:
				return RC_OK;
		}

		return RC_OK;
	}
	
	std::string COGLMappings::ErrorCodeToString(GLenum error)
	{
		switch (error)
		{
			case GL_INVALID_ENUM:
				return "GL_INVALID_ENUM";
			case GL_INVALID_VALUE:
				return "GL_INVALID_VALUE";
			case GL_OUT_OF_MEMORY:
				return "GL_OUT_OF_MEMORY";
			case GL_INVALID_OPERATION:
				return "GL_INVALID_OPERATION";
			case GL_STACK_UNDERFLOW:
				return "GL_STACK_UNDERFLOW";
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				return "GL_INVALID_FRAMEBUFFER_OPERATION";
			case GL_CONTEXT_LOST:
				return "GL_CONTEXT_LOST";
			case GL_NO_ERROR:
				return "GL_NO_ERROR";
		}

		return "";
	}

	GLenum COGLMappings::GetBlendFactorValue(E_BLEND_FACTOR_VALUE factor)
	{
		switch (factor)
		{
			case E_BLEND_FACTOR_VALUE::ZERO:
				return GL_ZERO;
			case E_BLEND_FACTOR_VALUE::ONE:
				return GL_ONE;
			case E_BLEND_FACTOR_VALUE::SOURCE_ALPHA:
				return GL_SRC_ALPHA;
			case E_BLEND_FACTOR_VALUE::ONE_MINUS_SOURCE_ALPHA:
				return GL_ONE_MINUS_SRC_ALPHA;
			case E_BLEND_FACTOR_VALUE::DEST_ALPHA:
				return GL_DST_ALPHA;
			case E_BLEND_FACTOR_VALUE::ONE_MINUS_DEST_ALPHA:
				return GL_ONE_MINUS_DST_ALPHA;
			case E_BLEND_FACTOR_VALUE::CONSTANT_ALPHA:
				return GL_CONSTANT_ALPHA;
			case E_BLEND_FACTOR_VALUE::ONE_MINUS_CONSTANT_ALPHA:
				return GL_ONE_MINUS_CONSTANT_ALPHA;
			case E_BLEND_FACTOR_VALUE::SOURCE_COLOR:
				return GL_SRC_COLOR;
			case E_BLEND_FACTOR_VALUE::ONE_MINUS_SOURCE_COLOR:
				return GL_ONE_MINUS_SRC_COLOR;
			case E_BLEND_FACTOR_VALUE::DEST_COLOR:
				return GL_DST_COLOR;
			case E_BLEND_FACTOR_VALUE::ONE_MINUS_DEST_COLOR:
				return GL_ONE_MINUS_DST_COLOR;
		}

		return GL_ZERO;
	}

	GLenum COGLMappings::GetBlendOpType(E_BLEND_OP_TYPE opType)
	{
		switch (opType)
		{
			case E_BLEND_OP_TYPE::ADD:
				return GL_FUNC_ADD;
			case E_BLEND_OP_TYPE::SUBT:
				return GL_FUNC_SUBTRACT;
			case E_BLEND_OP_TYPE::REVERSED_SUBT:
				return GL_FUNC_REVERSE_SUBTRACT;
		}

		return GL_FUNC_ADD;
	}

	GLenum COGLMappings::GetCubemapFace(E_CUBEMAP_FACE faceType)
	{
		switch (faceType)
		{
			case E_CUBEMAP_FACE::POSITIVE_X:
				return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
			case E_CUBEMAP_FACE::NEGATIVE_X:
				return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
			case E_CUBEMAP_FACE::POSITIVE_Y:
				return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
			case E_CUBEMAP_FACE::NEGATIVE_Y:
				return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
			case E_CUBEMAP_FACE::POSITIVE_Z:
				return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
			case E_CUBEMAP_FACE::NEGATIVE_Z:
				return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
		}

		return 0x0;
	}

	GLenum COGLMappings::GetTextureType(E_TEXTURE_IMPL_TYPE implType)
	{
		switch (implType)
		{
			case E_TEXTURE_IMPL_TYPE::CUBEMAP:
				return GL_TEXTURE_CUBE_MAP;
			case E_TEXTURE_IMPL_TYPE::TEXTURE_2D:
				return GL_TEXTURE_2D;
			case E_TEXTURE_IMPL_TYPE::TEXTURE_2D_ARRAY:
				return GL_TEXTURE_2D_ARRAY;
			case E_TEXTURE_IMPL_TYPE::TEXTURE_3D:
				return GL_TEXTURE_3D;
		}

		TDE2_UNREACHABLE();
		return 0;
	}

	GLenum COGLMappings::GetComparisonFunc(const E_COMPARISON_FUNC& func)
	{
		switch (func)
		{
			case E_COMPARISON_FUNC::NEVER:
				return GL_NEVER;
			case E_COMPARISON_FUNC::LESS:
				return GL_LESS;
			case E_COMPARISON_FUNC::EQUAL:
				return GL_EQUAL;
			case E_COMPARISON_FUNC::LESS_EQUAL:
				return GL_LEQUAL;
			case E_COMPARISON_FUNC::GREATER:
				return GL_GREATER;
			case E_COMPARISON_FUNC::NOT_EQUAL:
				return GL_NOTEQUAL;
			case E_COMPARISON_FUNC::GREATER_EQUAL:
				return GL_GEQUAL;
			case E_COMPARISON_FUNC::ALWAYS:
				return GL_ALWAYS;
		}

		return 0;
	}

	GLenum COGLMappings::GetStencilOpType(const E_STENCIL_OP& stencilOp)
	{
		switch (stencilOp)
		{
			case E_STENCIL_OP::ZERO:
				return GL_ZERO;
			case E_STENCIL_OP::REPLACE:
				return GL_REPLACE;
			case E_STENCIL_OP::INVERT:
				return GL_INVERT;
			case E_STENCIL_OP::INCR:
			case E_STENCIL_OP::INCR_SAT:
				return GL_INCR;
			case E_STENCIL_OP::DECR:
			case E_STENCIL_OP::DECR_SAT:
				return GL_DECR;
			case E_STENCIL_OP::KEEP:
				return GL_KEEP;
		}

		return 0;
	}

	GLenum COGLMappings::GetCullMode(const E_CULL_MODE& cullMode)
	{
		switch (cullMode)
		{
			case E_CULL_MODE::FRONT:
				return GL_FRONT;
			case E_CULL_MODE::BACK:
				return GL_BACK;
			case E_CULL_MODE::NONE:
				TDE2_ASSERT(false);
				return 0;
		}

		return 0;
	}
}