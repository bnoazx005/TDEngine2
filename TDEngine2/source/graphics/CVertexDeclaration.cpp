#include "../../include/graphics/CVertexDeclaration.h"


namespace TDEngine2
{
	CVertexDeclaration::CVertexDeclaration() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CVertexDeclaration::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CVertexDeclaration::AddElement(const TVertDeclElementDesc& elementDesc)
	{
		if (elementDesc.mSemanticType >= VEST_UNKNOWN ||
			elementDesc.mFormatType >= FT_UNKNOWN)
		{
			return RC_INVALID_ARGS;
		}

		mElements.emplace_back(elementDesc);

		return RC_OK;
	}

	E_RESULT_CODE CVertexDeclaration::AddInstancingDivisor(U32 index, U32 instancesPerData)
	{
		if (index >= mElements.size() ||
			(!mInstancingInfo.empty() && index < std::get<0/* element's index */>(mInstancingInfo.back()))) /// account correct ordering (from least to greatest)
		{
			return RC_INVALID_ARGS;
		}

		mInstancingInfo.emplace_back(index, instancesPerData);

		return RC_OK;
	}

	E_RESULT_CODE CVertexDeclaration::RemoveElement(U32 index)
	{
		if (index >= mElements.size())
		{
			return RC_INVALID_ARGS;
		}

		mElements.erase(mElements.cbegin() + index);

		return RC_OK;
	}

	U32 CVertexDeclaration::GetElementsCount() const
	{
		return mElements.size();
	}

	U32 CVertexDeclaration::GetStrideSize(U32 sourceIndex) const
	{
		U32 totalStrideSize = 0;
		
		for (auto currElement : mElements)
		{
			totalStrideSize = (currElement.mSource == sourceIndex) ? (totalStrideSize + _getFormatSize(currElement.mFormatType)) : totalStrideSize;
		}

		return totalStrideSize;
	}

	U32 CVertexDeclaration::_getFormatSize(E_FORMAT_TYPE format) const
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
}