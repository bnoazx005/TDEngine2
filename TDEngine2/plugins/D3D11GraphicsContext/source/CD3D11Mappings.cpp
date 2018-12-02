#include "./../include/CD3D11Mappings.h"


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	U32 CD3D11Mappings::GetAccessFlags(E_BUFFER_USAGE_TYPE type)
	{
		U32 accessFlags = 0x0;

		switch (type)
		{
			case BUT_DYNAMIC:
				accessFlags |= D3D11_CPU_ACCESS_WRITE;
				break;
		}

		return accessFlags;
	}
	
	D3D11_USAGE CD3D11Mappings::GetUsageType(E_BUFFER_USAGE_TYPE type)
	{
		switch (type)
		{
			case BUT_DYNAMIC:
				return D3D11_USAGE_DYNAMIC;
			case BUT_STATIC:
				return D3D11_USAGE_IMMUTABLE;
		}

		return D3D11_USAGE_DEFAULT;
	}

	DXGI_FORMAT CD3D11Mappings::GetDXGIFormat(E_FORMAT_TYPE format)
	{
		switch (format)
		{
			case FT_FLOAT1:
				return DXGI_FORMAT_R32_FLOAT;
			case FT_FLOAT2:
				return DXGI_FORMAT_R32G32_FLOAT;
			case FT_FLOAT3:
				return DXGI_FORMAT_R32G32B32_FLOAT;
			case FT_FLOAT4:
				return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case FT_BYTE1:
				return DXGI_FORMAT_R8_SINT;
			case FT_BYTE2:
				return DXGI_FORMAT_R8G8_SINT;
			case FT_BYTE4:
				return DXGI_FORMAT_R8G8B8A8_SINT;
			case FT_UBYTE1:
				return DXGI_FORMAT_R8_UINT;
			case FT_UBYTE2:
				return DXGI_FORMAT_R8G8_UINT;
			case FT_UBYTE4:
				return DXGI_FORMAT_R8G8B8A8_UINT;
			case FT_SHORT1:
				return DXGI_FORMAT_R16_SINT;
			case FT_SHORT2:
				return DXGI_FORMAT_R16G16_SINT;
			case FT_SHORT4:
				return DXGI_FORMAT_R16G16B16A16_SINT;
			case FT_USHORT1:
				return DXGI_FORMAT_R16_UINT;
			case FT_USHORT2:
				return DXGI_FORMAT_R16G16_UINT;
			case FT_USHORT4:
				return DXGI_FORMAT_R16G16B16A16_UINT;
			case FT_UINT1:
				return DXGI_FORMAT_R32_UINT;
			case FT_UINT2:
				return DXGI_FORMAT_R32G32_UINT;
			case FT_UINT3:
				return DXGI_FORMAT_R32G32B32_UINT;
			case FT_UINT4:
				return DXGI_FORMAT_R32G32B32A32_UINT;
			case FT_SINT1:
				return DXGI_FORMAT_R32_SINT;
			case FT_SINT2:
				return DXGI_FORMAT_R32G32_SINT;
			case FT_SINT3:
				return DXGI_FORMAT_R32G32B32_SINT;
			case FT_SINT4:
				return DXGI_FORMAT_R32G32B32A32_SINT;
			case FT_NORM_BYTE1:
				return DXGI_FORMAT_R8_UNORM;
			case FT_NORM_BYTE2:
				return DXGI_FORMAT_R8G8_UNORM;
			case FT_NORM_BYTE4_SRGB:
				return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			case FT_NORM_BYTE4:
				return DXGI_FORMAT_R8G8B8A8_UNORM;
			case FT_NORM_UBYTE1:
				return DXGI_FORMAT_R8_UNORM;
			case FT_NORM_UBYTE2:
				return DXGI_FORMAT_R8G8_UNORM;
			case FT_NORM_UBYTE4:
				return DXGI_FORMAT_R8G8B8A8_UNORM;
			case FT_NORM_SHORT1:
				return DXGI_FORMAT_R16_UNORM;
			case FT_NORM_SHORT2:
				return DXGI_FORMAT_R16G16_UNORM;
			case FT_NORM_SHORT4:
				return DXGI_FORMAT_R16G16B16A16_UNORM;
			case FT_NORM_USHORT1:
				return DXGI_FORMAT_R16_UNORM;
			case FT_NORM_USHORT2:
				return DXGI_FORMAT_R16G16_UNORM;
			case FT_NORM_USHORT4:
				return DXGI_FORMAT_R16G16B16A16_UNORM;
			case FT_FLOAT1_TYPELESS:
				return DXGI_FORMAT_R32_TYPELESS;
			case FT_FLOAT2_TYPELESS:
				return DXGI_FORMAT_R32G32_TYPELESS;
			case FT_FLOAT3_TYPELESS:
				return DXGI_FORMAT_R32G32B32_TYPELESS;
			case FT_FLOAT4_TYPELESS:
				return DXGI_FORMAT_R32G32B32A32_TYPELESS;
			case FT_D32:
				return DXGI_FORMAT_D32_FLOAT;
			case FT_UBYTE4_BGRA_UNORM:
				return DXGI_FORMAT_B8G8R8A8_UNORM;
		}

		return DXGI_FORMAT_UNKNOWN;
	}
	 
	U32 CD3D11Mappings::GetFormatSize(E_FORMAT_TYPE format)
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

	std::string CD3D11Mappings::GetShaderTargetVerStr(E_SHADER_STAGE_TYPE stageType, E_SHADER_FEATURE_LEVEL version)
	{
		std::string result;

		switch (stageType)
		{
			case SST_VERTEX:
				result = "vs_";
				break;
			case SST_PIXEL:
				result = "ps_";
				break;
			case SST_GEOMETRY:
				result = "gs_";
				break;
		}

		switch (version)
		{
			case SFL_3_0:
				return result + "3_0";
			case SFL_4_0:
				return result + "4_0";
			case SFL_5_0:
				return result + "5_0";
		}

		return "";
	}

	U32 CD3D11Mappings::GetNumOfChannelsOfFormat(E_FORMAT_TYPE format)
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
	
	const C8* CD3D11Mappings::GetSemanticTypeName(E_VERTEX_ELEMENT_SEMANTIC_TYPE semanticType)
	{
		switch (semanticType)
		{
			case VEST_POSITION:
				return "POSITION";
			case VEST_NORMAL:
				return "NORMAL";
			case VEST_TEXCOORDS:
				return "TEXCOORDS";
			case VEST_BINORMAL:
				return "BINORMAL";
			case VEST_TANGENT:
				return "TANGENT";
			case VEST_COLOR:
				return "COLOR";
		}

		return "UNKNOWN";
	}

	D3D11_PRIMITIVE_TOPOLOGY CD3D11Mappings::GetPrimitiveTopology(E_PRIMITIVE_TOPOLOGY_TYPE topologyType)
	{
		switch (topologyType)
		{
			case E_PRIMITIVE_TOPOLOGY_TYPE::PTT_POINT_LIST:
				return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
			case E_PRIMITIVE_TOPOLOGY_TYPE::PTT_LINE_LIST:
				return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
			case E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST:
				return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			case E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_STRIP:
				return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			case E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_FAN:
				return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
		}
	}

	DXGI_FORMAT CD3D11Mappings::GetIndexFormat(E_INDEX_FORMAT_TYPE indexFormatType)
	{
		switch (indexFormatType)
		{
			case IFT_INDEX16:
				return DXGI_FORMAT_R16_UINT;
			case IFT_INDEX32:
				return DXGI_FORMAT_R32_UINT;
		}

		return DXGI_FORMAT_UNKNOWN;
	}
}

#endif