#include "./../include/CD3D11VertexDeclaration.h"
#include "./../include/CD3D11Mappings.h"


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	CD3D11VertexDeclaration::CD3D11VertexDeclaration() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CD3D11VertexDeclaration::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}
		
		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D11VertexDeclaration::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}
		
		delete this;

		mIsInitialized = false;

		return RC_OK;
	}

	E_RESULT_CODE CD3D11VertexDeclaration::AddElement(const TVertDeclElementDesc& elementDesc)
	{
		TVertDeclElementDesc::TD3D11VertDeclElementDesc d3d11VertDecl = elementDesc.mD3D11;

		if (d3d11VertDecl.mFormatType >= FT_UNKNOWN ||
			d3d11VertDecl.mSemanticType >= VEST_UNKNOWN)
		{
			return RC_INVALID_ARGS;
		}

		mElements.emplace_back(d3d11VertDecl);

		return RC_OK;
	}

	E_RESULT_CODE CD3D11VertexDeclaration::RemoveElement(U32 index)
	{
		if (index >= mElements.size())
		{
			return RC_INVALID_ARGS;
		}

		mElements.erase(mElements.cbegin() + index);

		return RC_OK;
	}

	U32 CD3D11VertexDeclaration::GetElementsCount() const
	{
		return mElements.size();
	}

	U32 CD3D11VertexDeclaration::GetStrideSize() const
	{
		U32 totalStrideSize = 0;

		for (auto currElement : mElements)
		{
			totalStrideSize += CD3D11Mappings::GetFormatSize(currElement.mFormatType);
		}

		return totalStrideSize;
	}


	IVertexDeclaration* CreateD3D11VertexDeclaration(E_RESULT_CODE& result)
	{
		CD3D11VertexDeclaration* pVertexDeclInstance = new (std::nothrow) CD3D11VertexDeclaration();

		if (!pVertexDeclInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pVertexDeclInstance->Init();

		if (result != RC_OK)
		{
			delete pVertexDeclInstance;

			pVertexDeclInstance = nullptr;
		}

		return pVertexDeclInstance;
	}
}

#endif