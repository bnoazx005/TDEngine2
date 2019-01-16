#include "./../include/COGLMappings.h"


namespace TDEngine2
{
	GLenum COGLMappings::GetUsageType(E_BUFFER_USAGE_TYPE type)
	{
		if (type == BUT_DYNAMIC)
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
			case FT_D32:
				return GL_DEPTH_COMPONENT;
			case FT_NORM_BYTE1:
			case FT_BYTE1:
			case FT_NORM_UBYTE1:
			case FT_UBYTE1:
			case FT_NORM_SHORT1:
			case FT_SHORT1:
			case FT_NORM_USHORT1:
			case FT_USHORT1:
			case FT_UINT1:
			case FT_SINT1:
				return GL_RED;
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
	}

	GLenum COGLMappings::GetIndexFormat(E_INDEX_FORMAT_TYPE indexFormatType)
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

	GLint COGLMappings::GetMinFilterType(U32 filterValue)
	{
		E_FILTER_TYPE minFilterType = static_cast<E_FILTER_TYPE>(filterValue & static_cast<U32>(E_FILTER_FLAGS_MASKS::FFM_FILTER_MIN_MASK));
		E_FILTER_TYPE mipFilterType = static_cast<E_FILTER_TYPE>(filterValue & static_cast<U32>(E_FILTER_FLAGS_MASKS::FFM_FILTER_MIP_MASK));

		if (mipFilterType == E_FILTER_TYPE::FT_UNUSED)
		{
			switch (minFilterType)
			{
				case E_FILTER_TYPE::FT_POINT:
					return GL_NEAREST;
				case E_FILTER_TYPE::FT_BILINEAR:
					return GL_LINEAR;
				case E_FILTER_TYPE::FT_ANISOTROPIC: /// \todo anisotropic filter has not been implemented yet
					return 0x0;
				default:
					return GL_NEAREST;
			}
		}

		switch (minFilterType)
		{
			case E_FILTER_TYPE::FT_BILINEAR:
				switch (mipFilterType)
				{
					case E_FILTER_TYPE::FT_BILINEAR:
						return GL_LINEAR_MIPMAP_LINEAR;
					case E_FILTER_TYPE::FT_ANISOTROPIC: /// \todo anisotropic filter has not been implemented yet
						return 0x0;
					default:
						return GL_LINEAR_MIPMAP_NEAREST;
				}
			case E_FILTER_TYPE::FT_ANISOTROPIC: /// \todo anisotropic filter has not been implemented yet
				return 0x0;
			default:
				switch (mipFilterType)
				{
					case E_FILTER_TYPE::FT_BILINEAR:
						return GL_NEAREST_MIPMAP_LINEAR;
					case E_FILTER_TYPE::FT_ANISOTROPIC: /// \todo anisotropic filter has not been implemented yet
						return 0x0;
					default:
						return GL_NEAREST_MIPMAP_NEAREST;
				}
		}

		return GL_NEAREST;
	}

	GLint COGLMappings::GetMagFilterType(U32 filterValue)
	{
		E_FILTER_TYPE magFilterType = static_cast<E_FILTER_TYPE>(filterValue & static_cast<U32>(E_FILTER_FLAGS_MASKS::FFM_FILTER_MAG_MASK));
		E_FILTER_TYPE mipFilterType = static_cast<E_FILTER_TYPE>(filterValue & static_cast<U32>(E_FILTER_FLAGS_MASKS::FFM_FILTER_MIP_MASK));

		if (mipFilterType == E_FILTER_TYPE::FT_UNUSED)
		{
			switch (magFilterType)
			{
				case E_FILTER_TYPE::FT_POINT:
					return GL_NEAREST;
				case E_FILTER_TYPE::FT_BILINEAR:
					return GL_LINEAR;
				case E_FILTER_TYPE::FT_ANISOTROPIC: /// \todo anisotropic filter has not been implemented yet
					return 0x0;
				default:
					return GL_NEAREST;
			}
		}

		switch (magFilterType)
		{
			case E_FILTER_TYPE::FT_BILINEAR:
				switch (mipFilterType)
				{
					case E_FILTER_TYPE::FT_BILINEAR:
						return GL_LINEAR_MIPMAP_LINEAR;
					case E_FILTER_TYPE::FT_ANISOTROPIC: /// \todo anisotropic filter has not been implemented yet
						return 0x0;
					default:
						return GL_LINEAR_MIPMAP_NEAREST;
				}
			case E_FILTER_TYPE::FT_ANISOTROPIC: /// \todo anisotropic filter has not been implemented yet
				return 0x0;
			default:
				switch (mipFilterType)
				{
					case E_FILTER_TYPE::FT_BILINEAR:
						return GL_NEAREST_MIPMAP_LINEAR;
					case E_FILTER_TYPE::FT_ANISOTROPIC: /// \todo anisotropic filter has not been implemented yet
						return 0x0;
					default:
						return GL_NEAREST_MIPMAP_NEAREST;
				}
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
}