#include "../include/CVulkanMappings.h"
#include <utils/Utils.h>


namespace TDEngine2
{
	VmaMemoryUsage CVulkanMappings::GetUsageType(E_BUFFER_USAGE_TYPE type)
	{
		switch (type)
		{
			case E_BUFFER_USAGE_TYPE::BUT_DEFAULT:
				return VMA_MEMORY_USAGE_GPU_ONLY;
			case E_BUFFER_USAGE_TYPE::BUT_DYNAMIC:
				return VMA_MEMORY_USAGE_CPU_TO_GPU;
			case E_BUFFER_USAGE_TYPE::BUT_STATIC:
				return VMA_MEMORY_USAGE_GPU_ONLY;
		}

		TDE2_UNREACHABLE();
		return VMA_MEMORY_USAGE_UNKNOWN;
	}

	VkSampleCountFlagBits CVulkanMappings::GetSamplesCount(U32 samplesCount)
	{
		switch (samplesCount)
		{
			case 1: 
				return VK_SAMPLE_COUNT_1_BIT;
			case 2:
				return VK_SAMPLE_COUNT_2_BIT;
			case 4: 
				return VK_SAMPLE_COUNT_4_BIT;
			case 8: 
				return VK_SAMPLE_COUNT_8_BIT;
			case 16: 
				return VK_SAMPLE_COUNT_16_BIT;
			case 32: 
				return VK_SAMPLE_COUNT_32_BIT;
			case 64: 
				return VK_SAMPLE_COUNT_64_BIT;
		}

		return VK_SAMPLE_COUNT_1_BIT;
	}

	/*GLenum CVulkanMappings::GetBufferMapAccessType(E_BUFFER_MAP_TYPE type)
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
	
	GLenum CVulkanMappings::GetPixelDataFormat(E_FORMAT_TYPE format)
	{
		switch (format)
		{
			case FT_FLOAT1:
			case FT_FLOAT1_TYPELESS:
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

	U32 CVulkanMappings::GetNumOfChannelsOfFormat(E_FORMAT_TYPE format)
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

	GLenum CVulkanMappings::GetBaseTypeOfFormat(E_FORMAT_TYPE format)
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

	GLint CVulkanMappings::GetTypeSize(GLenum type)
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

	bool CVulkanMappings::IsFormatNormalized(E_FORMAT_TYPE format)
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

	GLenum CVulkanMappings::GetPrimitiveTopology(E_PRIMITIVE_TOPOLOGY_TYPE topologyType)
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

	GLenum CVulkanMappings::GetIndexFormat(E_INDEX_FORMAT_TYPE indexFormatType)
	{
		switch (indexFormatType)
		{
			case IFT_INDEX16:
				return GL_UNSIGNED_SHORT;
			case IFT_INDEX32:
				return GL_UNSIGNED_INT;
		}

		return 0;
	}

	GLint CVulkanMappings::GetMinFilterType(E_TEXTURE_FILTER_TYPE filterValue, bool useMipMaps)
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

	GLint CVulkanMappings::GetMagFilterType(E_TEXTURE_FILTER_TYPE filterValue)
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

	GLint CVulkanMappings::GetTextureAddressMode(E_ADDRESS_MODE_TYPE addressMode)
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
	}*/

	VkFormat CVulkanMappings::GetInternalFormat(E_FORMAT_TYPE format)
	{
		switch (format)
		{
			case FT_FLOAT1:
			case FT_FLOAT1_TYPELESS:
				return VK_FORMAT_R32_SFLOAT;
			case FT_FLOAT2:
			case FT_FLOAT2_TYPELESS:
				return VK_FORMAT_R32G32_SFLOAT;
			case FT_FLOAT3:
			case FT_FLOAT3_TYPELESS:
				return VK_FORMAT_R32G32B32_SFLOAT;
			case FT_FLOAT4:
			case FT_FLOAT4_TYPELESS:
				return VK_FORMAT_R32G32B32A32_SFLOAT;
			case FT_NORM_BYTE1:
			case FT_BYTE1:
				return VK_FORMAT_R8_SNORM;
			case FT_NORM_BYTE2:
			case FT_BYTE2:
				return VK_FORMAT_R8G8_SNORM;
			case FT_NORM_BYTE4:
			case FT_BYTE4:
				return VK_FORMAT_R8G8B8A8_SNORM;
			case FT_NORM_UBYTE1:
			case FT_UBYTE1:
				return VK_FORMAT_R8_UNORM;
			case FT_NORM_UBYTE2:
			case FT_UBYTE2:
				return VK_FORMAT_R8G8_UNORM;
			case FT_NORM_UBYTE4:
			case FT_UBYTE4:
				return VK_FORMAT_R8G8B8A8_UNORM;
			case FT_UBYTE4_BGRA_UNORM:
				return VK_FORMAT_B8G8R8A8_UNORM;
			case FT_NORM_SHORT1:
			case FT_SHORT1:
				return VK_FORMAT_R16_SNORM;
			case FT_NORM_SHORT2:
			case FT_SHORT2:
				return VK_FORMAT_R16G16_SNORM;
			case FT_NORM_SHORT4:
			case FT_SHORT4:
				return VK_FORMAT_R16G16B16A16_SNORM;
			case FT_NORM_USHORT1:
			case FT_USHORT1:
				return VK_FORMAT_R16_UNORM;
			case FT_NORM_USHORT2:
			case FT_USHORT2:
				return VK_FORMAT_R16G16_UNORM;
			case FT_NORM_USHORT4:
			case FT_USHORT4:
				return VK_FORMAT_R16G16B16A16_UNORM;
			case FT_UINT1:
				return VK_FORMAT_R32_UINT;
			case FT_UINT2:
				return VK_FORMAT_R32G32_UINT;
			case FT_UINT3:
				return VK_FORMAT_R32G32B32_UINT;
			case FT_UINT4:
				return VK_FORMAT_R32G32B32A32_UINT;
			case FT_SINT1:
				return VK_FORMAT_R32_SINT;
			case FT_SINT2:
				return VK_FORMAT_R32G32_SINT;
			case FT_SINT3:
				return VK_FORMAT_R32G32B32_SINT;
			case FT_SINT4:
				return VK_FORMAT_R32G32B32A32_SINT;
			case FT_D32:
				return VK_FORMAT_D32_SFLOAT;
		}

		TDE2_UNREACHABLE();
		return VK_FORMAT_R8G8B8A8_UNORM;
	}

	VkShaderStageFlagBits CVulkanMappings::GetShaderStageType(E_SHADER_STAGE_TYPE shaderStageType)
	{
		switch (shaderStageType)
		{
			case SST_VERTEX:
				return VK_SHADER_STAGE_VERTEX_BIT;
			case SST_PIXEL:
				return VK_SHADER_STAGE_FRAGMENT_BIT;
			case SST_GEOMETRY:
				return VK_SHADER_STAGE_GEOMETRY_BIT;
			case SST_COMPUTE:
				return VK_SHADER_STAGE_COMPUTE_BIT;
		}

		TDE2_UNREACHABLE();
		return VK_SHADER_STAGE_VERTEX_BIT;
	}

	E_RESULT_CODE CVulkanMappings::GetErrorCode(VkResult resultCode)
	{
		// \todo Support more flags 
		return VK_SUCCESS == resultCode ? RC_OK : RC_FAIL;
	}

	/*
	std::string CVulkanMappings::ErrorCodeToString(GLenum error)
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

	GLenum CVulkanMappings::GetBlendFactorValue(E_BLEND_FACTOR_VALUE factor)
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

	GLenum CVulkanMappings::GetBlendOpType(E_BLEND_OP_TYPE opType)
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

	GLenum CVulkanMappings::GetCubemapFace(E_CUBEMAP_FACE faceType)
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

	GLenum CVulkanMappings::GetComparisonFunc(const E_COMPARISON_FUNC& func)
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

	GLenum CVulkanMappings::GetStencilOpType(const E_STENCIL_OP& stencilOp)
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

	GLenum CVulkanMappings::GetCullMode(const E_CULL_MODE& cullMode)
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
	}*/
}