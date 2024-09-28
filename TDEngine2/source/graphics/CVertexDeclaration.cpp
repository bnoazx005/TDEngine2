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
		return static_cast<U32>(mElements.size());
	}

	U32 CVertexDeclaration::GetStrideSize(U32 sourceIndex) const
	{
		U32 totalStrideSize = 0;
		
		for (auto currElement : mElements)
		{
			totalStrideSize = (currElement.mSource == sourceIndex) ? (totalStrideSize + CFormatUtils::GetFormatSize(currElement.mFormatType)) : totalStrideSize;
		}

		return totalStrideSize;
	}
}