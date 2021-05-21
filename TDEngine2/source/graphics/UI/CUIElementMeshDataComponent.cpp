#include "../../../include/graphics/UI/CUIElementMeshDataComponent.h"


namespace TDEngine2
{
	struct TUIElementMeshDataArchiveKeys
	{
	};


	CUIElementMeshData::CUIElementMeshData() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CUIElementMeshData::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mTextureResourceId = TResourceId::Invalid;

		mIsFontMesh = false;
		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CUIElementMeshData::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}
		
		return RC_OK;
	}

	E_RESULT_CODE CUIElementMeshData::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	void CUIElementMeshData::ResetMesh()
	{
		mVertices.clear();
		mIndices.clear();
	}

	void CUIElementMeshData::AddVertex(const TUIElementsVertex& vertex)
	{
		mVertices.emplace_back(vertex);
	}

	void CUIElementMeshData::AddIndex(U16 value)
	{
		mIndices.push_back(value);
	}

	void CUIElementMeshData::SetTextMeshFlag(bool value)
	{
		mIsFontMesh = value;
	}

	E_RESULT_CODE CUIElementMeshData::SetTextureResourceId(TResourceId resourceId)
	{
		if (TResourceId::Invalid == resourceId)
		{
			return RC_INVALID_ARGS;
		}

		mTextureResourceId = resourceId;

		return RC_OK;
	}

	const CUIElementMeshData::TVertexArray& CUIElementMeshData::GetVertices() const
	{
		return mVertices;
	}

	const CUIElementMeshData::TIndexArray& CUIElementMeshData::GetIndices() const
	{
		return mIndices;
	}

	bool CUIElementMeshData::IsTextMesh() const
	{
		return mIsFontMesh;
	}

	TResourceId CUIElementMeshData::GetTextureResourceId() const
	{
		return mTextureResourceId;
	}


	IComponent* CreateUIElementMeshData(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CUIElementMeshData, result);
	}


	CUIElementMeshDataFactory::CUIElementMeshDataFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CUIElementMeshDataFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CUIElementMeshDataFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CUIElementMeshDataFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TUIElementMeshDataParameters* params = static_cast<const TUIElementMeshDataParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreateUIElementMeshData(result);
	}

	IComponent* CUIElementMeshDataFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateUIElementMeshData(result);
	}

	TypeId CUIElementMeshDataFactory::GetComponentTypeId() const
	{
		return CUIElementMeshData::GetTypeId();
	}


	IComponentFactory* CreateUIElementMeshDataFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CUIElementMeshDataFactory, result);
	}
}